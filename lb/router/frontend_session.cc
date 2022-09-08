#include "frontend_session.h"

#include "backend_session.h"
#include "load_balancer.h"
#include "config.h"

using namespace lb;
using namespace kanon;
using namespace http;

FrontendSession::FrontendSession(LoadBalancer *lb, TcpConnectionPtr const &conn)
  : lb_(lb)
  , codec_(conn)
{
  codec_.SetRequestCallback([this, _conn = conn](TcpConnectionPtr const &conn,
                                                 HttpRequest &request,
                                                 TimeStamp) {
    auto index = lb_->tl_index_.value();
    auto *backend = GetPointer(lb_->pt_backends_[index]);
    
    BackendSession *current_session = nullptr;
    { 
    MutexGuard g(backend->lock);
    
    for (;;) {
      if (backend->current == backend->backends.size()) backend->current = 0;
      current_session = GetPointer(backend->backends[backend->current]);
      auto &header_map = request.headers;
      auto iter = header_map.find("Host");
      if (iter != header_map.end())
        iter->second = lbconfig().backend_addrs[backend->current++].ToIpPort();
      if (current_session->Send(_conn, codec_, request)) {
        LOG_INFO << "Select the server index: " << backend->current-1;
        break;
      }
    } // lock
    }
  });
}
