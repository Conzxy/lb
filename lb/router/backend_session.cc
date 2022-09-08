#include "backend_session.h"

#include "kanon/log/logger.h"
#include "lb/http/http_request.h"
#include "lb/http/http_request_codec.h"

#undef PARSER_DEBUG
#define PARSER_DEBUG 1

#if PARSER_DEBUG
#include <iostream>
#endif

using namespace lb;
using namespace kanon;
using namespace http;

#define LOG_SEND_DIRECTION(frontend, backend)                                  \
  LOG_INFO << "Send" << frontend->GetPeerAddr().ToIpPort() << "->"             \
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
#if PARSER_DEBUG
#endif
        for (;;) {
          switch (parser_.Parse(buffer)) {
          case HttpResponseParser::PARSE_OK: {
            LOG_DEBUG << "Parse OK";
            assert(!frontends_.empty());
            auto frontend = GetPointer(frontends_.back());
            if (!parser_.IsChunked()) frontends_.pop_back();

            std::string response = buffer.RetrieveAsString(parser_.index());
            parser_.ResetIndex();

#if PARSER_DEBUG
            LOG_DEBUG << "Response: ";
#endif
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

void BackendSession::Send(TcpConnectionPtr const &frontend,
                          OutputBuffer &buffer)
{
  LOG_SEND_DIRECTION(frontend, conn_);
  if (conn_) {
    frontends_.push_front(frontend);
    conn_->Send(buffer);
  } else {
    LOG_WARN << "Backend connection is not established";
  }
}

void BackendSession::Send(TcpConnectionPtr const &frontend, Buffer &buffer)
{
  OutputBuffer output;
  output.Append(buffer.ToStringView());

  Send(frontend, output);
}

void BackendSession::Send(TcpConnectionPtr const &frontend,
                          HttpRequestCodec &codec, HttpRequest &request)
{
  LOG_SEND_DIRECTION(frontend, conn_);
  if (conn_) {
    frontends_.push_front(frontend);
    codec.Send(conn_, request);
  } else {
    LOG_WARN << "Backend connection is not established";
  }
}
