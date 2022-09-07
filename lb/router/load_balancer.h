#ifndef LB_LOAD_BALANCER_H__
#define LB_LOAD_BALANCER_H__

#include "kanon/net/user_server.h"
#include "kanon/thread/thread_local.h"

#include "backend_session.h"

namespace lb {

class BackendSession;

class LoadBalancer : kanon::noncopyable {
  friend class FrontendSession;

  struct PerThreadData {
    kanon::MutexLock lock;
    std::vector<std::unique_ptr<BackendSession>> backends;
    size_t current = 0;
  };

 public:
  LoadBalancer(EventLoop *loop, InetAddr const &listen_addr,
               std::vector<InetAddr> init_servers);

  void SetLoopNum(uint32_t num)
  {
    pt_backends_.reserve(num);
    server_.SetLoopNum(num);
  }
  
  void Listen()
  {
    server_.StartRun();
  }

 private:
  TcpServer server_;
  std::vector<InetAddr> backends_;
  std::vector<std::unique_ptr<PerThreadData>> pt_backends_;
  kanon::ThreadLocal<size_t> tl_index_;
};
} // namespace lb

#endif // LB_LOAD_BALANCER_H__
