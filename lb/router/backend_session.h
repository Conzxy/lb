#ifndef LB_BACKEND_SESSION__
#define LB_BACKEND_SESSION__

#include "kanon/net/user_client.h"
#include "kanon/util/noncopyable.h"
#include <deque>

#include "lb/http/http_response_parser.h"

namespace http {

struct HttpRequest;
}

namespace lb {

class HttpRequestCodec;

class BackendSession : kanon::noncopyable {
 public:
  BackendSession(EventLoop *loop, InetAddr const &backend_addr,
                 std::string const &name);

  bool Send(TcpConnectionPtr const &frontend, HttpRequestCodec &codec,
            http::HttpRequest &request);

  void Connect()
  {
    backend_->Connect();
  }
  void Disconnect()
  {
    backend_->Disconnect();
  }

 private:
  TcpClientPtr backend_;
  TcpConnectionPtr conn_;
  http::HttpResponseParser parser_;
  std::deque<TcpConnectionPtr> frontends_;
};
} // namespace lb
#endif // LB_BACKEND_SESSION__
