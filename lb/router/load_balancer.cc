#include "load_balancer.h"

#include "backend_session.h"
#include "frontend_session.h"

using namespace kanon;
using namespace lb;

LoadBalancer::LoadBalancer(EventLoop *loop, InetAddr const &listen_addr,
                           std::vector<InetAddr> init_servers)
  : server_(loop, listen_addr, "LoadBalancer")
  , backends_(std::move(init_servers))
{
  server_.SetThreadInitCallback([this](EventLoop *io_loop) {
    PerThreadData *data = new PerThreadData;
    auto &index = tl_index_.value();
    index = pt_backends_.size();
    for (size_t i = 0; i < backends_.size(); ++i) {
      data->backends.emplace_back(new BackendSession(
          io_loop, backends_[i],
          "BackendSession #" + std::to_string(index) +
              "-" + std::to_string(i)));
      data->backends.back()->Connect();
    }
    pt_backends_.emplace_back(data);
  });

#define LOG_ENDPOINT(conn)                                                     \
  LOG_INFO << conn->GetPeerAddr().ToIpPort() << "->"                           \
           << conn->GetLocalAddr().ToIpPort()

  server_.SetConnectionCallback([this](TcpConnectionPtr const &conn) {
    if (conn->IsConnected()) {
      LOG_ENDPOINT(conn) << " UP";
      conn->SetContext(new FrontendSession(this, conn));
    } else {
      LOG_ENDPOINT(conn) << " DOWN";
      delete *AnyCast<FrontendSession*>(conn->GetContext());
    }
  });

  // server_.SetMessageCallback(
  //     [this](TcpConnectionPtr const &conn, Buffer &buffer, TimeStamp) {
  //     });
}
