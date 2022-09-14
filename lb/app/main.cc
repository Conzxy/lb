#include <takina/takina.h>

#include "lb/router/config.h"
#include "lb/router/load_balancer.h"
#include "lb/router/option.h"
#include "kanon/log/async_log.h"

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

  LoadBalancer lb(&loop, InetAddr(lboption().port), lbconfig().backends);
  lb.SetLoopNum(lboption().io_loop_num);

  lb.Listen();

  if (lboption().silent) {
    LOG_INFO << "Running in silent mode";
    EnableAllLog(false);
  }
  
  if (!lboption().log_dir.empty()) {
    kanon::SetupAsyncLog("load_balancer", 64 * (1 << 20), lboption().log_dir);
  }
  
  loop.StartLoop();
}
