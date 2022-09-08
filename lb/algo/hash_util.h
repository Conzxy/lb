#ifndef LB_ALGO_HASH_UTIL_H__
#define LB_ALGO_HASH_UTIL_H__

#include <stdint.h>
#include <string>
#include <xxhash.h>

namespace lb {

inline uint64_t Hash(std::string const &key, int seed) noexcept
{
  return XXH64(key.data(), key.size(), seed);
}

inline uint64_t Hash(std::string const &key) noexcept
{
  return Hash(key, 0);
}

} // namespace lb

#endif
