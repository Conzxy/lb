#include "config.h"

#include "kanon/log/logger.h"

#include <toml.hpp>

using namespace lb;
using namespace kanon;

bool lb::ParseLbConfig(StringView filename, LoadBalancerConfig &config)
{
  try {
    auto contents = std::move(toml::parse(filename.ToString()).as_table());
    
    std::string lb_algo =
        std::move(toml::get_or<std::string>(contents.at("balancing_algorithm"), "round-robin"));

    if (lb_algo == "round-robin") {
      config.bl_algo_type = BlAlgoType::ROUND_ROBIN;
    } else if (lb_algo == "consistent_hashing") {
      config.bl_algo_type = BlAlgoType::CONSISTENT_HASHING;
    }

    auto servers = std::move(contents.at("servers").as_array());

    for (auto &_server_info : servers) {
      auto server_info = std::move(_server_info.as_table());
      auto &endpoint = toml::get<std::string>(server_info["endpoint"]);
      LOG_DEBUG << endpoint;
      config.backend_addrs.emplace_back(endpoint);
    }
  } catch (std::exception const &ex) {
    fprintf(stderr, "%s", ex.what());
    return false;
  }
  return true;
}

LoadBalancerConfig &lb::lbconfig()
{
  static LoadBalancerConfig config;
  return config;
}

static char const *BlAlgoType2String(BlAlgoType type) noexcept
{
  switch (type) {
    case lb::BlAlgoType::ROUND_ROBIN:
      return "Round robin";
    case lb::BlAlgoType::CONSISTENT_HASHING:
      return "Consistent hashing";
  }
  return "Unknown balancing algorithm";
}

void lb::DebugLbConfig(LoadBalancerConfig const &config)
{
  LOG_DEBUG << "========== Load balancer configuration ==========";
  LOG_DEBUG << "Balancing algorithm: " << BlAlgoType2String(config.bl_algo_type);
  LOG_DEBUG << "servers: ";
  for (auto &server : config.backend_addrs) {
    LOG_DEBUG << server.ToIpPort();
  }
}
