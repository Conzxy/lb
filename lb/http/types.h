#ifndef KANON_HTTP_TYPES_H
#define KANON_HTTP_TYPES_H

#include <unordered_map>
#include <string>
#include <string.h>

namespace http {

struct CaseInsensitiveEqual {
  inline bool operator()(std::string const &x, std::string const &y) const noexcept 
  {
    return strcasecmp(x.c_str(), y.c_str()) == 0;
  }
};

using HeaderMap = std::unordered_map<std::string, std::string, std::hash<std::string>, CaseInsensitiveEqual>;
using HeaderType = HeaderMap::value_type;
using ArgsMap = std::unordered_map<std::string, std::string>;

} // namespace http

#endif // KANON_HTTP_TYPES_H
