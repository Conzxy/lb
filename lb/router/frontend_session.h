#ifndef LB_FRONTEND_SESSION_H__
#define LB_FRONTEND_SESSION_H__

#include "kanon/net/tcp_connection.h"
#include "kanon/util/noncopyable.h"
#include "lb/http/http_request_codec.h"

namespace lb {

class LoadBalancer;

class FrontendSession : kanon::noncopyable {
 public:
  FrontendSession(LoadBalancer *lb, kanon::TcpConnectionPtr const &conn);

 private:
  LoadBalancer *lb_;
  HttpRequestCodec codec_;
};
} // namespace lb
#endif // LB_FRONTEND_SESSION_H__
