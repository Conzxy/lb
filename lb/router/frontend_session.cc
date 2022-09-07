#include "frontend_session.h"

#include "backend_session.h"
#include "load_balancer.h"

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

    MutexGuard g(backend->lock);

    if (backend->current == backend->backends.size()) backend->current = 0;
    auto current_session = GetPointer(backend->backends[backend->current++]);
    current_session->Send(_conn, codec_, request);
  });
}
