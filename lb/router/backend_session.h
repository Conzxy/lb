#ifndef LB_BACKEND_SESSION__
#define LB_BACKEND_SESSION__

#include "kanon/net/user_client.h"
#include "kanon/util/noncopyable.h"
#include "kanon/util/optional.h"
#include <deque>

#include "lb/http/http_response_parser.h"
#include "lb/router/config.h"

namespace http {

struct HttpRequest;
}

namespace lb {

class HttpRequestCodec;
class LoadBalancer;
class FrontendSession;

class BackendSession : kanon::noncopyable {
 public:
  BackendSession(EventLoop *loop, InetAddr const &backend_addr,
                 std::string const &name, 
                 TcpConnectionPtr const &frontend,
                 FrontendSession &frontend_session,
                 int index, LoadBalancer &lb);
  
  ~BackendSession() noexcept;

  bool Send(TcpConnectionPtr const &frontend, HttpRequestCodec &codec,
            http::HttpRequest &request);
  
  InetAddr const &GetAddr();

  void Connect()
  {
    backend_->Connect();
  }
  void Disconnect()
  {
    backend_->Disconnect();
  }
  
  TcpConnectionPtr const &GetConnection() noexcept { return conn_; }
  bool IsFailed() const noexcept { return is_fail_; }

  void HandleFailed();
  void HandleRecover();

  void HandleDisconnet();
 private:
  TcpClientPtr backend_;
  TcpConnectionPtr conn_;
  http::HttpResponseParser parser_;
  TcpConnectionPtr frontend_;

  /* Used for health checking */
  kanon::optional<kanon::TimerId> fail_timer_;
  kanon::optional<kanon::TimerId> conn_timer_;
  bool is_fail_ = false;
  int total_request_num_{0};
  int success_request_num_{0};
  int index_;
  LoadBalancer *lb_;
};
} // namespace lb
#endif // LB_BACKEND_SESSION__
