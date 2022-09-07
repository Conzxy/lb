#include "lb/router/load_balancer.h"

#include "lb/router/config.h"
#include "lb/router/option.h"

#include <takina/takina.h>

using namespace lb;
using namespace kanon;

int main(int argc, char *argv[])
{
  EventLoop loop;
  
  std::string errmsg; 
  RegisterOptions(lboption());
  if (!takina::Parse(argc, argv, &errmsg)) {
    LOG_FATAL << errmsg;
  }

  ParseLbConfig(lboption().config_filename, lbconfig());
  LoadBalancer lb(&loop, InetAddr(lboption().port), lbconfig().backend_addrs);
  
  lb.Listen();  
  loop.StartLoop();  
}
