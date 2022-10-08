#ifndef LB_LOAD_BALANCER_H__
#define LB_LOAD_BALANCER_H__

#include "kanon/net/user_server.h"
#include "backend_session.h"
#include "kanon/thread/thread_local.h"
#include "kanon/thread/mutex_lock.h"

#include "lb/algo/consistent_hash.h"
#include "config.h"

namespace lb {

class BackendSession;
class FrontendSession;

class LoadBalancer : kanon::noncopyable {
  friend class FrontendSession;
  friend class BackendSession;

 public:
  LoadBalancer(EventLoop *loop, InetAddr const &listen_addr,
               std::vector<ServerConfig> init_servers);

  ~LoadBalancer() noexcept;

  void SetLoopNum(uint32_t num)
  {
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
  void InitBalancingAlgorithm();

  TcpServer server_;

  kanon::MutexLock backend_lock_;
  std::vector<ServerConfig> backends_ GUARDED_BY(backend_lock_);

  size_t current_ GUARDED_BY(backend_lock_) = 0;
  std::unordered_set<size_t> failed_nodes_ GUARDED_BY(backend_lock_);
  
  ConsistentHash chash_ GUARDED_BY(backend_lock_);
};
} // namespace lb

#endif // LB_LOAD_BALANCER_H__
