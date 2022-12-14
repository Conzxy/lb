#ifndef LB_FRONTEND_SESSION_H__
#define LB_FRONTEND_SESSION_H__

#include "kanon/net/tcp_connection.h"
#include "kanon/util/noncopyable.h"
#include "lb/http/http_request_codec.h"

#include "backend_session.h"
#include "load_balancer.h"

namespace lb {

class LoadBalancer;

class FrontendSession : public kanon::noncopyable {
  using RequestQueue = std::vector<http::HttpRequest>;

 public:
  FrontendSession(kanon::TcpConnectionPtr const &conn,
                  LoadBalancer &lb);
  
  ~FrontendSession() noexcept; 
  
  FrontendSession(FrontendSession &&) noexcept = default;
  FrontendSession &operator=(FrontendSession &&) noexcept = default;

  RequestQueue &GetRequestQueue() noexcept
  {
    return request_queue_;
  }
  
  HttpRequestCodec &GetRequestCodec() noexcept
  {
    return codec_;
  }
  
  void SetBackendSession(BackendSession *b) noexcept
  {
    backend_ = b;
  }
  
  void SetBackendConnection(TcpConnectionPtr const &c) noexcept
  {
    backend_conn_ = c;
  }

  TcpConnectionPtr &GetBackendConnection() noexcept
  {
    return backend_conn_;
  }

 private:
  LoadBalancer *lb_;
  HttpRequestCodec codec_;
  BackendSession *backend_;
  TcpConnectionPtr backend_conn_;
  RequestQueue request_queue_;
};
} // namespace lb
#endif // LB_FRONTEND_SESSION_H__
