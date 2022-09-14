#include "option.h"

#include "takina/takina.h"

using namespace lb;

void lb::RegisterOptions(LbOption &option)
{
  takina::AddDescription("Http load balancer");
  takina::AddUsage("./load-balancer [OPTIONS]");
  takina::AddOption({"p", "port", "Port number(default: 9999)", "PORT"}, &option.port);
  takina::AddOption({"c", "config", "Filename of config(default: ./lb.conf)", "CONFIG_FILE_NAME"}, &option.config_filename);
  takina::AddOption({"l", "loop-num", "The number of the IO loop(default: 0)", "LOOP_NUM"}, &option.io_loop_num);
  takina::AddOption({"s", "silent", "Running in silent mode(default: disable)"}, &option.silent);
  takina::AddOption({"d", "log-dir", "Directory that store logs(default: "", i.e. disable)", "DIR"}, &option.log_dir);
}

LbOption &lb::lboption()
{
  static LbOption option;
  return option;
}

