#include <toml.hpp>
#include <iostream>

int main()
{
  auto data = toml::parse("example.toml");
  auto servers = toml::get<toml::array>(toml::get<toml::table>(data).at("servers"));
  
  for (auto &_server : servers) {
    auto server = toml::get<toml::array>(_server);
    std::cout << toml::get<std::string>(server[0]) << ":" << toml::get<int64_t>(server[1]) << "\n";
  } 

}
