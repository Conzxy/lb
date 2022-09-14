#ifndef LB_LOAD_BALANCER_H__
#define LB_LOAD_BALANCER_H__

#include "kanon/net/user_server.h"
#include "backend_session.h"
#include "kanon/thread/thread_local.h"

#include "lb/algo/consistent_hash.h"
#include "config.h"

namespace lb {

class BackendSession;
class FrontendSession;

class LoadBalancer : kanon::noncopyable {
  friend class FrontendSession;
  friend class BackendSession;

  // struct PerThreadData {
  //   kanon::MutexLock lock;
  //   std::vector<std::unique_ptr<BackendSession>> backends;
  //   size_t current = 0;
  //   ConsistentHash chash;

  //   ~PerThreadData() noexcept;
  // };
 public:

  LoadBalancer(EventLoop *loop, InetAddr const &listen_addr,
               std::vector<ServerConfig> init_servers);

  ~LoadBalancer() noexcept;

  void SetLoopNum(uint32_t num)
  {
    // pt_backends_.reserve(num);
    server_.SetLoopNum(num);
  }
  
  void Listen()
  {
    server_.StartRun();
  }
  
  ServerConfig const &GetServerConfig(size_t index) const
  {
    return backends_[index];
  }

 private:
  TcpServer server_;

  kanon::MutexLock backend_lock_;
  std::vector<ServerConfig> backends_;
  size_t current_ = 0;
  ConsistentHash chash_;
  std::unordered_map<std::string, std::unique_ptr<BackendSession>> backend_map_;
  // std::vector<std::unique_ptr<PerThreadData>> pt_backends_;
  // kanon::ThreadLocal<size_t> tl_index_;
};
} // namespace lb

#endif // LB_LOAD_BALANCER_H__
