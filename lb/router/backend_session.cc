#include "backend_session.h"

#include "kanon/log/logger.h"
#include "lb/http/http_request.h"
#include "lb/http/http_request_codec.h"

using namespace lb;
using namespace kanon;
using namespace http;

#define LOG_SEND_DIRECTION(frontend, backend)                                  \
  LOG_INFO << "Send direction: " << frontend->GetPeerAddr().ToIpPort() << "->"             \
           << backend->GetPeerAddr().ToIpPort();

BackendSession::BackendSession(EventLoop *loop, InetAddr const &backend_addr,
                               std::string const &name)
  : backend_(NewTcpClient(loop, backend_addr, name))
{

  backend_->SetConnectionCallback([this](TcpConnectionPtr const &conn) {
    if (conn->IsConnected()) {
      LOG_INFO << "Connect to backend " << conn->GetPeerAddr().ToIpPort()
               << " successfully";
      conn_ = conn;
    } else {
      LOG_INFO << "Disconnect to backend " << conn->GetPeerAddr().ToIpPort();
      conn_.reset();
    }
  });

  backend_->SetMessageCallback(
      [this](TcpConnectionPtr const &conn, Buffer &buffer, TimeStamp) {
        for (;;) {
          switch (parser_.Parse(buffer)) {
          case HttpResponseParser::PARSE_OK: {
            LOG_DEBUG << "Parse OK";
            assert(!frontends_.empty());
            auto frontend = GetPointer(frontends_.back());
            if (!parser_.IsChunked()) frontends_.pop_back();

            std::string response = buffer.RetrieveAsString(parser_.index());
            parser_.ResetIndex();

            LOG_DEBUG << "Response length = " << response.size();

            LOG_SEND_DIRECTION(conn, frontend);
            frontend->Send(response);
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
          // FIXME
          // Error handling
        }
      });

  backend_->EnableRetry();
}

bool BackendSession::Send(TcpConnectionPtr const &frontend,
                          HttpRequestCodec &codec, HttpRequest &request)
{
  LOG_SEND_DIRECTION(frontend, conn_);
  if (conn_) {
    frontends_.push_front(frontend);
    codec.Send(conn_, request);
    return true;
  } else {
    LOG_WARN << "Backend connection is not established";
    return false;
  }
}
