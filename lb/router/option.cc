#include "option.h"

#include "takina/takina.h"

using namespace lb;

void lb::RegisterOptions(LbOption &option)
{
  takina::AddDescription("Http load balancer");
  takina::AddUsage("./load-balancer [OPTIONS]");
  takina::AddOption({"p", "port", "Port number(default: 9999)"}, &option.port);
  takina::AddOption({"c", "config", "Filename of config(default: ./lb.conf)"}, &option.config_filename);
  takina::AddOption({"l", "loop-num", "The number of the IO loop(default: 0)"}, &option.io_loop_num);
}

LbOption &lb::lboption()
{
  static LbOption option;
  return option;
}

