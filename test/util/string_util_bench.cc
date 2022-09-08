#include <benchmark/benchmark.h>

#include <string>
#include <absl/strings/str_cat.h>
#include "lb/util/string_util.h"

static std::string buf;

static void
BM_Absl_StrCat(benchmark::State &state)
{
  buf.resize(state.range(0));
  for (auto _ : state) {
    auto s = absl::StrCat(buf, buf, buf, buf, buf);
  }
}

static void
BM_MyStrCat(benchmark::State &state)
{
  buf.resize(state.range(0)); 
  for (auto _ : state) {
    auto s = util::StrCat(buf, buf, buf, buf, buf);
  }
}

static void
BM_StdStrCat(benchmark::State &state)
{
  buf.resize(state.range(0));
  for (auto _ : state) {
    auto s = std::string(buf) + buf + buf + buf + buf;
  }
}

static void
BM_MyStrAppend(benchmark::State &state)
{
  buf.resize(state.range(0));
  for (auto _ : state) {
    std::string s = buf;
    util::StrAppend(s, buf, buf, buf, buf);
  }
}

static void
BM_Absl_StrAppend(benchmark::State &state)
{
  buf.resize(state.range(0));
  for (auto _ : state) {
    std::string s = buf;
    absl::StrAppend(&s, buf, buf, buf, buf);
  }
}
#define BM_STRCAT_DEFINE(func, name) \
  BENCHMARK(func)->Name(name)->RangeMultiplier(2)->Range(10, 10000)

BM_STRCAT_DEFINE(BM_MyStrCat, "util::StrCat");
BM_STRCAT_DEFINE(BM_StdStrCat, "std::string operator+");
BM_STRCAT_DEFINE(BM_Absl_StrCat, "absl::StrCat");
BM_STRCAT_DEFINE(BM_MyStrAppend, "util::StrAppend");
BM_STRCAT_DEFINE(BM_Absl_StrAppend, "absl::StrAppend");
BENCHMARK_MAIN();
