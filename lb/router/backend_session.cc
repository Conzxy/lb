#include "backend_session.h"

#include "lb/http/http_request_codec.h"
#include "lb/http/http_request.h"

using namespace lb;
using namespace kanon;
using namespace http;

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
        if (parser_.Parse(buffer) == HttpResponseParser::PARSE_OK) {
          assert(!frontends_.empty());
          auto frontend = std::move(frontends_.back());
          frontends_.pop_back();

          std::string response = buffer.RetrieveAsString(parser_.index());
          parser_.ResetIndex();
          frontend->Send(response);
        }
        // FIXME 
        // Error handling
      });

  backend_->EnableRetry();
}

#define LOG_SEND_DIRECTION(frontend, backend)                                  \
  LOG_INFO << "Send" << frontend->GetPeerAddr().ToIpPort() << "->"             \
           << backend->GetPeerAddr().ToIpPort();

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
                          HttpRequestCodec &codec,
                          HttpRequest &request)
{
  LOG_SEND_DIRECTION(frontend, conn_);
  if (conn_) {
    frontends_.push_front(frontend);
    codec.Send(conn_, request);
  } else {
    LOG_WARN << "Backend connection is not established";
  }
}
