#include <takina/takina.h>

#include "lb/router/config.h"
#include "lb/router/load_balancer.h"
#include "lb/router/option.h"

using namespace lb;
using namespace kanon;

int main(int argc, char *argv[]) {
  EventLoop loop;

  std::string errmsg;
  RegisterOptions(lboption());
  if (!takina::Parse(argc, argv, &errmsg)) {
    LOG_FATAL << errmsg;
  }

  if (!ParseLbConfig(lboption().config_filename, lbconfig())) {
    return 0;
  }

  LoadBalancer lb(&loop, InetAddr(lboption().port), lbconfig().backend_addrs);
  lb.SetLoopNum(lboption().io_loop_num);

  lb.Listen();
  loop.StartLoop();
}
