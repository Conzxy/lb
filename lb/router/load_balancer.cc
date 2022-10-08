#include "load_balancer.h"

#include "backend_session.h"
#include "frontend_session.h"
#include "lb/router/config.h"
#include "lb/util/string_util.h"

using namespace kanon;
using namespace lb;

#define LOG_ENDPOINT(conn)                                                     \
  LOG_INFO << conn->GetPeerAddr().ToIpPort() << "->"                           \
           << conn->GetLocalAddr().ToIpPort()

LoadBalancer::~LoadBalancer() noexcept 
{

}

LoadBalancer::LoadBalancer(EventLoop *loop, InetAddr const &listen_addr,
                           std::vector<ServerConfig> init_servers)
  : server_(loop, listen_addr, "LoadBalancer")
  , backends_(std::move(init_servers))
{
  InitBalancingAlgorithm();

  server_.SetConnectionCallback([this](TcpConnectionPtr const &conn) {
    if (conn->IsConnected()) {
      LOG_ENDPOINT(conn) << " UP";

      /* Select a proper backend and allocate it to a frontend */
      int index = 0;
      InetAddr backend_addr;
      {
        MutexGuard g(backend_lock_);
        switch (lbconfig().bl_algo_type) {
          case BlAlgoType::ROUND_ROBIN:
          {
            if (failed_nodes_.size() == backends_.size()) {
              /* FIXME Don't abort the process */
              LOG_FATAL << "No backend avaliable";
            }

            for (;;) {
              if (current_ == backends_.size())
                  current_ = 0;
              if (failed_nodes_.find(current_) != failed_nodes_.end())
                current_++;
              else break;
            }
            index = current_;
            ++current_;
          } break;
          case BlAlgoType::CONSISTENT_HASHING:
          {
            index = chash_.QueryServer(conn->GetPeerAddr().ToIp());
          } break;
        }
        backend_addr = backends_[index].addr;
      } /* Critical section */
  
      /* FIXME Consider there are no backends avaliable */
      LOG_INFO << "Selected index = " << index;
      auto frontend_session = new FrontendSession(conn, *this);
      conn->SetContext(*frontend_session);

      /* backend_session is running in the same loop as frontend_session */

      /* The BackendSession is not owned by the according FrontendSession
       * i.e. their lifetime are not coupled.
       */
      auto backend_name = util::StrCat(conn->GetName(), "-backend");
      auto backend_session =  new BackendSession(conn->GetLoop(), backend_addr, backend_name, conn, *frontend_session, index, *this);
      frontend_session->SetBackendSession(backend_session);
    } else {
      LOG_TRACE << "Frontend " << conn->GetName();
      LOG_ENDPOINT(conn) << " DOWN";
      
      // auto frontend_session = *AnyCast<FrontendSession*>(conn->GetContext());
      auto frontend_session = AnyCast<FrontendSession>(conn->GetContext());

      /* check if backend is active */
      // auto backend_conn = frontend_session->GetBackendConnection();
      auto backend_conn = frontend_session->GetBackendConnection();
      LOG_DEBUG << "Frontend conn ref cnt = " << conn.use_count();

      if (backend_conn && backend_conn->IsConnected()) {
        LOG_TRACE << "FrontendSession is disconnected early";
        LOG_DEBUG << "Backend conn ref cnt = " << backend_conn.use_count();
        // backend_session.Disconnect();
        backend_conn->ShutdownWrite();
      }

      delete frontend_session;
    }
  });
}

void LoadBalancer::InitBalancingAlgorithm() 
{
  // Init balancing algorithm configuration
  switch (lbconfig().bl_algo_type) {
    case BlAlgoType::ROUND_ROBIN:
    {

    } break;

    case BlAlgoType::CONSISTENT_HASHING:
    {
      for (size_t i = 0; i < backends_.size(); ++i) {
        auto &config = backends_[i];
        chash_.AddServer(i, config.name, config.vnode);
      }
    }
  }

}
