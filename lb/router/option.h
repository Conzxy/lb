#ifndef LB_ROUTER_OPTION_H__
#define LB_ROUTER_OPTION_H__

#include <string>

namespace lb {

struct LbOption {
  int port = 9999;
  std::string config_filename = "./lb.conf";
  int io_loop_num = 0;
};

void RegisterOptions(LbOption &option);

LbOption &lboption();

} // namespace lb
#endif // LB_ROUTER_OPTION_H__
