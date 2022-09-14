#include "load_balancer.h"

#include "backend_session.h"
#include "frontend_session.h"
#include "lb/router/config.h"
#include "lb/util/string_util.h"

using namespace kanon;
using namespace lb;

LoadBalancer::~LoadBalancer() noexcept 
{

}

// LoadBalancer::PerThreadData::~PerThreadData() noexcept
// {

// }

LoadBalancer::LoadBalancer(EventLoop *loop, InetAddr const &listen_addr,
                           std::vector<ServerConfig> init_servers)
  : server_(loop, listen_addr, "LoadBalancer")
  , backends_(std::move(init_servers))
{
  // pt_backends_.reserve(1);

  // server_.SetThreadInitCallback([this](EventLoop *io_loop) {
  //   PerThreadData *data = new PerThreadData;
  //   auto &index = tl_index_.value();
  //   index = pt_backends_.size();
  //   pt_backends_.emplace_back(data);
  //   for (size_t i = 0; i < backends_.size(); ++i) {
  //     data->backends.emplace_back(new BackendSession(
  //         io_loop, backends_[i].addr,
  //         util::StrCat("BackendSession #", std::to_string(index), 
  //             "-", std::to_string(i)), i, *pt_backends_[index]));
  //     LOG_INFO << "backend[" << i << "] start connect";
  //     data->backends.back()->Connect();
  //   }
  // });
  
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

#define LOG_ENDPOINT(conn)                                                     \
  LOG_INFO << conn->GetPeerAddr().ToIpPort() << "->"                           \
           << conn->GetLocalAddr().ToIpPort()

  server_.SetConnectionCallback([this](TcpConnectionPtr const &conn) {
    if (conn->IsConnected()) {
      LOG_ENDPOINT(conn) << " UP";
      int index = 0;
      switch (lbconfig().bl_algo_type) {
        case BlAlgoType::ROUND_ROBIN:
        {
          if (current_ == backends_.size())
              current_ = 0;
          index = current_;
          ++current_;
        } break;
        case BlAlgoType::CONSISTENT_HASHING:
        {
          index = chash_.QueryServer(conn->GetPeerAddr().ToIpPort());
        }
      }
      // LOG_DEBUG << "tl_index = " << tl_index_.value();
      // conn->SetContext(new FrontendSession(this, conn, *pt_backends_[tl_index_.value()]));
      auto backend_name = util::StrCat(conn->GetName(), " backend");
      auto frontend_session = new FrontendSession(conn, index, *this);
      auto backend_session =  new BackendSession(conn->GetLoop(), backends_[index].addr, backend_name, conn, *frontend_session, index, *this);
      backend_map_.emplace(backend_name, backend_session);
      frontend_session->SetBackendSession(backend_session);
      conn->SetContext(frontend_session);
    } else {
      LOG_DEBUG << conn->GetName() << " disconnection handler";
      LOG_ENDPOINT(conn) << " DOWN";
      
      auto frontend_session = *AnyCast<FrontendSession*>(conn->GetContext());
      auto backend_conn = frontend_session->GetBackendConnection();
      LOG_DEBUG << "Frontend conn ref cnt = " << conn.use_count();
      if (backend_conn && backend_conn->IsConnected()) {
        LOG_TRACE << "FrontendSession is disconnected early";
        LOG_DEBUG << "Backend conn ref cnt = " << backend_conn.use_count();
        // backend_session.Disconnect();
        backend_conn->ShutdownWrite();
        // conn->GetLoop()->QueueToLoop([backend_conn]() {
        //   if (backend_conn->IsConnected())
        //     backend_conn->ForceClose();
        // });
      } 
      delete frontend_session;
      // else {
      //   LOG_DEBUG << "Frontend conn ref cnt = " << conn.use_count();
        // conn->GetLoop()->QueueToLoop([frontend_session, conn]() {
        //   delete frontend_session;
        //   conn->SetContext(nullptr);
        //   LOG_ERROR << "Frontend conn ref cnt = " << conn.use_count();
        // });
      // }
    }
  });
}

