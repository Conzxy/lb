#include "backend_session.h"

#include "config.h"
#include "kanon/log/logger.h"
#include "lb/http/http_request.h"
#include "lb/http/http_request_codec.h"
#include "load_balancer.h"
#include "frontend_session.h"

using namespace lb;
using namespace kanon;
using namespace http;

#define LOG_SEND_DIRECTION(frontend, backend)                                  \
  LOG_INFO << "Send direction: " << frontend->GetPeerAddr().ToIpPort() << "->" \
           << backend->GetPeerAddr().ToIpPort();

BackendSession::BackendSession(EventLoop *loop, InetAddr const &backend_addr,
                               std::string const &name,
                               TcpConnectionPtr const &frontend,
                               FrontendSession &frontend_session,
                               int index,
                               LoadBalancer &lb)
  : backend_(NewTcpClient(loop, backend_addr, name))
  , conn_()
  , frontend_(frontend)
  , index_(index)
  , lb_(&lb)
{
  backend_->SetConnectionCallback([this, &frontend_session](TcpConnectionPtr const &conn) {
    if (conn->IsConnected()) {
      LOG_INFO << "Connect to backend " << conn->GetPeerAddr().ToIpPort()
               << " successfully";

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
        auto &header_map = request.headers;
        /* FIXME Necessary? */
        auto iter = header_map.find("Host");
        if (iter != header_map.end()) {
          iter->second = conn->GetPeerAddr().ToIpPort();
        }

        Send(frontend_, frontend_session.GetRequestCodec(), request);
      }
      request_queue.clear();
    } else {
      LOG_INFO << "Disconnect to backend [" << conn->GetName()  << "] - [" << conn->GetPeerAddr().ToIpPort() << "]";
      /* If connection is setted, indicates the frontend is disconnected
       * early and backend is forced to disconnect also. */
      if (conn_) {
        LOG_DEBUG << "conn_ = " << conn_.get();
        LOG_DEBUG << "conn = " << conn.get();
        assert(conn_ == conn);
        conn_.reset();
        
        LOG_INFO << "Backend client ref = " << backend_.use_count();
        LOG_DEBUG << "frontend_ ref cnt = " << frontend_.use_count();
        if (frontend_->IsConnected()) {
          LOG_TRACE << "BackendSession is disconnected early";
          // frontend_->ForceClose();
          frontend_->ShutdownWrite();
        } 
        
        frontend_.reset();
      }

      int count = 0;
      size_t total_backend_cnt = 0;
      {
      MutexGuard g(lb_->backend_map_lock_);
      count = lb_->backend_map_.erase(conn->GetName());
      total_backend_cnt = lb_->backend_map_.size();
      }
      LOG_DEBUG << "backend_map count: " << total_backend_cnt;
      assert(count == 1); KANON_UNUSED(count);
    }
  });

  backend_->SetMessageCallback(
      [this](TcpConnectionPtr const &conn, Buffer &buffer, TimeStamp) {
        // if (fail_timer_) {
        //   conn->GetLoop()->CancelTimer(*fail_timer_);
        // }
        for (;;) {
          switch (parser_.Parse(buffer)) {
            case HttpResponseParser::PARSE_OK: {
              LOG_DEBUG << "Parse OK";
              // assert(!frontends_.empty());
              // auto frontend = GetPointer(frontends_.back());
              // if (!parser_.IsChunked()) frontends_.pop_back();


              LOG_SEND_DIRECTION(conn, frontend_);
              // std::string response = buffer.RetrieveAsString(parser_.index());
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
  // backend_->EnableRetry();

  // Passive health checking
  // fail_timer_ = loop->RunEvery(
  //     [this]() {
  //       // if (is_fail_ && success_request_num_ == total_request_num_) {
  //       //   is_fail_ = false;
  //       //   total_request_num_ = success_request_num_ = 0;
  //       //   LOG_INFO << "The server " << lbconfig().backends[index_].name
  //       //            << "is recoverd";
  //       // }
  //       // else if (!frontends_.empty() &&
  //       //            total_request_num_ - success_request_num_ >= 1)
  //       // {
  //       //   is_fail_ = true;
  //       //   LOG_INFO << "The server " << lbconfig().backends[index_].name
  //       //            << " is failed";
  //       // }
  //     },
  //     double(lbconfig().backends[index_].fail_timeout) / 1000);

}

BackendSession::~BackendSession() noexcept
{
  LOG_INFO << "Backend session is destroyed";
}

bool BackendSession::Send(TcpConnectionPtr const &frontend,
                          HttpRequestCodec &codec, HttpRequest &request)
{
  if (conn_) {
    LOG_SEND_DIRECTION(frontend, conn_);
    // frontends_.push_front(frontend);
    codec.Send(conn_, request);
    total_request_num_++;
    return true;
  } 

  LOG_TRACE << "Connection is not established";
  LOG_TRACE << "The request is cached in the queue";
  LOG_TRACE << "Consume it when connection is established";
  return false;
}

InetAddr const &BackendSession::GetAddr()
{
  return backend_->GetServerAddr();
}
