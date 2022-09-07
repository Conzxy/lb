#include <benchmark/benchmark.h>

#include <string>
#include <absl/strings/str_cat.h>
#include "lb/util/string_util.h"

static std::string buf;

static void
BM_ABSL_STRCAT(benchmark::State &state)
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

#define BM_STRCAT_DEFINE(func, name) \
  BENCHMARK(func)->Name(name)->RangeMultiplier(10)->Range(10, 100000)

BM_STRCAT_DEFINE(BM_MyStrCat, "util::StrCat large");
BM_STRCAT_DEFINE(BM_StdStrCat, "std::string operator+ large");
BM_STRCAT_DEFINE(BM_ABSL_STRCAT, "absl::StrCat large");
BENCHMARK_MAIN();
