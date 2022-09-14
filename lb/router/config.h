#ifndef LB_CONFIG_H__
#define LB_CONFIG_H__

#include "kanon/net/inet_addr.h"

namespace lb {

/**
 * Balancing algorithm
 */
enum class BlAlgoType : unsigned char {
  ROUND_ROBIN = 0,
  CONSISTENT_HASHING,
};

struct ServerConfig {
  kanon::InetAddr addr;
  std::string name;
  int vnode;
  int fail_timeout;
};

/**
 * Config metadata of load balancer
 */
struct LoadBalancerConfig {
  BlAlgoType bl_algo_type;
  std::vector<ServerConfig> backends;
};

/**
 * Parse the config file
 * \param filename The filename of the config file
 * \param[out] config The config data
 * \return 
 *   true -- success
 */
bool ParseLbConfig(kanon::StringView filename, LoadBalancerConfig &config);

/**
 * Print the infomation of load balancer config
 */
void DebugLbConfig(LoadBalancerConfig const &config);

/**
 * Singleton object
 * Threas safe(Since c++11)
 */
LoadBalancerConfig &lbconfig();

} // namespace lb

#endif // LB_CONFIG_H__
