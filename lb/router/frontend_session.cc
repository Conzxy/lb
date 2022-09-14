#include "frontend_session.h"

#include "backend_session.h"
#include "config.h"
#include "load_balancer.h"

#include "lb/util/string_util.h"

using namespace lb;
using namespace kanon;
using namespace http;

FrontendSession::FrontendSession(TcpConnectionPtr const &conn,
                                 int index,
                                 LoadBalancer &lb)
  : lb_(&lb)
  , codec_(conn)
  // , backend_(conn->GetLoop(), backend_addr,
  //            util::StrCat(conn->GetName(), " backend"), conn, *this, index, lb)
// , pt_data_(&pt_data)
{

  codec_.SetRequestCallback([this, _conn = conn](TcpConnectionPtr const &conn,
                                                 HttpRequest &request,
                                                 TimeStamp) {
    // auto index = lb_->tl_index_.value();
    // auto *backend = GetPointer(lb_->pt_backends_[index]);
    // BackendSession *current_session = nullptr;
    //
    // auto &header_map = request.headers;
    // auto iter = header_map.find("Host");
    // if (iter != header_map.end()) {
    //   // iter->second = lbconfig().backends[backend->current++].addr.ToIpPort();
    //   /* FIXME Test */
    //   iter->second = backend_->GetAddr().ToIpPort();
    // }

    // if (request.version == HttpVersion::kHttp11) {
    //   iter = header_map.find("Connection");
    //   if (iter != header_map.end()) {
    //     iter->second = "Keep-Alive";
    //   }
    // }

    if (backend_->Send(_conn, codec_, request)) {
      LOG_TRACE << "Send OK";
    } else {
      request_queue_.push_back(std::move(request));
    }

    // {
    // MutexGuard g(backend->lock);

    // switch (lbconfig().bl_algo_type) {
    //   case BlAlgoType::ROUND_ROBIN: {
    //     for (;;) {
    //       if (backend->current == backend->backends.size())
    //         backend->current = 0;
    //       current_session = GetPointer(backend->backends[backend->current]);
    //       if (current_session->Send(_conn, codec_, request)) {
    //         LOG_INFO << "Select the server index: " << backend->current - 1;
    //         break;
    //       }
    //     }
    //   } break;

    //   case BlAlgoType::CONSISTENT_HASHING: {
    //     auto ip = conn->GetPeerAddr().ToIp();
    //     LOG_DEBUG << "Consistent hashing Ip address: " << ip;

    //     auto index = pt_data_->chash.QueryServer(ip);
    //     current_session = GetPointer(backend->backends[index]);
    //     if (current_session->Send(_conn, codec_, request)) {
    //       LOG_INFO << "Select the server index: " << index;
    //     }
    //   } break;
    // }
    // } // lock
  });
}
