#include "lb/config/config.h"

using namespace lb;

int main()
{
  ParseLbConfig("/root/lb/bin/lb.conf", lbconfig());
  DebugLbConfig(lbconfig());
}
