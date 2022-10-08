#include "backend_session.h"

#include "config.h"
#include "frontend_session.h"
#include "kanon/log/logger.h"
#include "lb/http/http_request.h"
#include "lb/http/http_request_codec.h"
#include "load_balancer.h"

using namespace lb;
using namespace kanon;
using namespace http;

#define LOG_SEND_DIRECTION(frontend, backend)                                  \
  LOG_INFO << "Send direction: " << frontend->GetPeerAddr().ToIpPort() << "->" \
           << backend->GetPeerAddr().ToIpPort();

#define FAIL_TIMEOUT(index) \
  double(lbconfig().backends[index].fail_timeout) / 1000.0

BackendSession::BackendSession(EventLoop *loop, InetAddr const &backend_addr,
                               std::string const &name,
                               TcpConnectionPtr const &frontend,
                               FrontendSession &frontend_session, int index,
                               LoadBalancer &lb)
  : backend_(NewTcpClient(loop, backend_addr, name))
  , conn_()
  , frontend_(frontend)
  , index_(index)
  , lb_(&lb)
{
  conn_timer_ = loop->RunAfter([this]() {
    if (!conn_) {
      /* FIXME Get next valid index in backends */
      backend_->Stop();
      HandleFailed();
      HandleDisconnet(); 
      /* delete self */
      delete this;
    }
  }, FAIL_TIMEOUT(index_));

  backend_->SetConnectionCallback(
      [this, &frontend_session](TcpConnectionPtr const &conn) {
        if (conn->IsConnected()) {
          LOG_INFO << "Connect to backend " << conn->GetPeerAddr().ToIpPort()
                   << " successfully";

          conn->SetContext(*this);

          /* FIXME assert? */
          if (conn_timer_) {
            conn->GetLoop()->CancelTimer(*conn_timer_);
          }

          /* If frontend is disconnected before
           * the backend connection is established,
           * just disconnect peer. */
          if (!frontend_->IsConnected()) {
            Disconnect();
            return;
          }

          /* Binding connection(frontend, backend) */
          conn_ = conn;

          /* Safe
           * If backend is disconnected, will return early */
          frontend_session.SetBackendConnection(conn_);

          auto &request_queue = frontend_session.GetRequestQueue();
          for (auto &request : request_queue) {
            Send(frontend_, frontend_session.GetRequestCodec(), request);
          }
          request_queue.clear();
        } else {
          HandleDisconnet();
          auto backend_session = AnyCast<BackendSession>(conn->GetContext());
          delete backend_session;
        }
      });

  backend_->SetMessageCallback(
      [this](TcpConnectionPtr const &conn, Buffer &buffer, TimeStamp) {
        if (fail_timer_) {
          conn->GetLoop()->CancelTimer(*fail_timer_);
          fail_timer_.SetInvalid();
        }
        for (;;) {
          switch (parser_.Parse(buffer)) {
            case HttpResponseParser::PARSE_OK: {
              LOG_DEBUG << "Parse OK";
              LOG_SEND_DIRECTION(conn, frontend_);
              LOG_DEBUG << "Response length = " << parser_.index();
              frontend_->Send(buffer.GetReadBegin(), parser_.index());
              buffer.AdvanceRead(parser_.index());
              parser_.ResetIndex();
              success_request_num_++;
            } break;

            case HttpResponseParser::PARSE_ERR: {
              LOG_ERROR << "Response parse error";
              return;
            }

            case HttpResponseParser::PARSE_SHORT: {
              LOG_DEBUG << "Parse short";
              return;
            }
          }
        }
      });

  backend_->Connect();

  /* Passive health checking */
  fail_timer_ = loop->RunEvery(
      [this, loop]() {
        if (is_fail_ && success_request_num_ == total_request_num_) {
          is_fail_ = false;
          total_request_num_ = success_request_num_ = 0;
          LOG_INFO << "The server " << lbconfig().backends[index_].name
                   << " is recoverd";
          HandleRecover();
        } else if (total_request_num_ - success_request_num_ >= 1) {
          is_fail_ = true;
          LOG_INFO << "The server " << lbconfig().backends[index_].name
                   << " is failed";
          HandleFailed();
          Disconnect();

          loop->RunAfter([this]() {
            is_fail_ = true;
            HandleRecover();
          }, FAIL_TIMEOUT(index_));
        }
      }, FAIL_TIMEOUT(index_));
}

BackendSession::~BackendSession() noexcept
{
  if (fail_timer_ && conn_) {
    conn_->GetLoop()->CancelTimer(*fail_timer_);
  }

  if (conn_timer_) {
    backend_->GetLoop()->CancelTimer(*conn_timer_);
  }

  LOG_INFO << "Backend session is destroyed";
}

bool BackendSession::Send(TcpConnectionPtr const &frontend,
                          HttpRequestCodec &codec, HttpRequest &request)
{
  if (conn_) {
    LOG_SEND_DIRECTION(frontend, conn_);

    /* Rewrite some header to fit the load balancer property */
    auto &header_map = request.headers;
    /* FIXME Necessary? */
    auto iter = header_map.find("Host");
    if (iter != header_map.end()) {
      iter->second = conn_->GetPeerAddr().ToIpPort();
    }

    header_map["Connection"] = "Close";

    codec.Send(conn_, request);
    total_request_num_++;

    return true;
  }

  LOG_TRACE << "Connection is not established";
  LOG_TRACE << "The request is cached in the queue";
  LOG_TRACE << "Consume it when connection is established";
  return false;
}

InetAddr const &BackendSession::GetAddr() { return backend_->GetServerAddr(); }

void BackendSession::HandleFailed()
{
  MutexGuard g(lb_->backend_lock_);
  switch (lbconfig().bl_algo_type) {
    case BlAlgoType::ROUND_ROBIN:
    {
      lb_->failed_nodes_.insert(index_);
    } break;

    case BlAlgoType::CONSISTENT_HASHING:
    {
      lb_->chash_.RemoveServer(lb_->backends_[index_].name);
    } break;
  }

}

void BackendSession::HandleRecover()
{
  MutexGuard g(lb_->backend_lock_);
  switch (lbconfig().bl_algo_type) {
    case BlAlgoType::ROUND_ROBIN:
    {
      lb_->failed_nodes_.erase(index_);
    } break;

    case BlAlgoType::CONSISTENT_HASHING:
    {
      auto const &config = lb_->backends_[index_];
      lb_->chash_.AddServer(index_, config.name, config.vnode);
    } break;
  }
}

void BackendSession::HandleDisconnet()
{
  LOG_INFO << "Disconnect to backend [" << conn_->GetName() << "] - ["
           << conn_->GetPeerAddr().ToIpPort() << "]";
  /* If connection is setted, indicates the frontend is disconnected
   * early and backend is forced to disconnect also. */
  if (conn_) {
    conn_.reset();

    LOG_INFO << "Backend client ref = " << backend_.use_count();
    LOG_DEBUG << "frontend_ ref cnt = " << frontend_.use_count();
    if (frontend_->IsConnected()) {
      LOG_TRACE << "BackendSession is disconnected early";
      frontend_->ShutdownWrite();
    }

    frontend_.reset();
  }
}
