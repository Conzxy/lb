#ifndef LB_UTIL_STRING_UTIL_H_
#define LB_UTIL_STRING_UTIL_H_

#include <string>
#include <tuple>

#include "kanon/string/string_view.h"

namespace util {

namespace detail {

inline size_t Strlen(std::string const &str) noexcept
{
  return str.size();
}

inline size_t Strlen(char const *str) noexcept
{
  return strlen(str);
}

inline size_t Strlen(kanon::StringView const &str) noexcept
{
  return str.size();
}

template <typename S, typename... Args>
size_t StrSize_impl(S const &head, Args &&... strs) noexcept
{
  return Strlen(head) + StrSize_impl(strs...);
}

template <typename S>
size_t StrSize_impl(S const &head) noexcept
{
  return Strlen(head);
}

template <typename... Args>
size_t StrSize(Args &&... args)
{
  return StrSize_impl(args...);
}

template <typename... Args>
void StrAppend_impl(std::string &str, size_t &size, size_t &index,
                    std::string const &head, Args &&... strs)
{
  size += head.size();
  StrAppend_impl(str, size, index, strs...);
  assert(index - head.size() >= 0);
  index -= head.size();
  memcpy(&str[index], head.data(), head.size());
}

template <typename... Args>
void StrAppend_impl(std::string &str, size_t &size, size_t &index,
                    kanon::StringView const &head, Args &&... strs)
{
  size += head.size();
  StrAppend_impl(str, size, index, strs...);
  assert(index - head.size() >= 0);
  index -= head.size();
  memcpy(&str[index], head.data(), head.size());
}

template <typename... Args>
void StrAppend_impl(std::string &str, size_t &size, size_t &index,
                    char const* head, Args &&... strs)
{
  auto len = strlen(head);
  size += len;
  StrAppend_impl(str, size, index, strs...);
  assert(index - len >= 0);
  index -= len;
  memcpy(&str[index], head, len);
}

template <>
inline void StrAppend_impl<>(std::string &str, size_t &size, size_t &index,
                             kanon::StringView const &head)
{
  size += head.size();
  str.resize(size);
  index = size - head.size();
  memcpy(&str[index], head.data(), head.size());
}

template <>
inline void StrAppend_impl<>(std::string &str, size_t &size, size_t &index,
                             std::string const &head)
{
  size += head.size();
  str.resize(size);
  index = size - head.size();
  memcpy(&str[index], head.data(), head.size());
}

template <>
inline void StrAppend_impl<>(std::string &str, size_t &size, size_t &index,
                             char const *head)
{
  auto len = strlen(head);
  size += len;
  str.resize(size);
  index = size - len;
  memcpy(&str[index], head, len);
}

} // namespace detail

template <typename... Args>
void StrAppend(std::string &str, Args &&... strs)
{
  size_t size = 0;
  size_t index = 0;
  detail::StrAppend_impl(str, size, index, strs...);
}

template <typename... Args>
std::string StrCat(Args &&... args)
{
  std::string ret;
  StrAppend(ret, args...);
  return ret;
}
} // namespace util

#endif // LB_UTIL_STRING_UTIL_H_
