#include "frontend_session.h"

#include "backend_session.h"
#include "config.h"
#include "load_balancer.h"

#include "lb/util/string_util.h"

using namespace lb;
using namespace kanon;
using namespace http;

FrontendSession::FrontendSession(TcpConnectionPtr const &conn,
                                 LoadBalancer &lb)
  : lb_(&lb)
  , codec_(conn)
  , backend_conn_()
{
  codec_.SetRequestCallback(
      [this](TcpConnectionPtr const &conn, HttpRequest &request, TimeStamp) {
        // auto backend_conn_sptr = backend_conn_.lock();
        // auto backend_sptr = backend_.lock();

        /* If backend is down, just cache */
        if (backend_conn_ && backend_conn_->IsConnected() &&
            backend_->Send(conn, codec_, request)) {
          LOG_TRACE << "Send OK";
        } else {
          request_queue_.push_back(std::move(request));
        }
      });
}

FrontendSession::~FrontendSession() noexcept
{
  LOG_INFO << "Frontend session is destroyed";
}
