//
// Created by Lukas Varhol on 12/7/2025.
//
#include <benchmark/benchmark.h>
#include "../include/logging/Logger.h"


// COMPARE LOGGING VS NO LOGGING
static double doTradingWorkNoLogging(int iterations) {
  volatile double sum = 0.0;
  for (int i = 0; i < iterations; ++i) {
    sum += (i * 2.34) / (i + 1.32);
    sum *= 0.999;
  }
  return sum;
}

static double doTradingWorkWithLogging(int iterations) {
  volatile double sum = 0.0;
  for (int i = 0; i < iterations; ++i) {
    sum += (i * 2.34) / (i + 1.32);
    sum *= 0.999;
    LOG.info(TRADES, "sum = " + std::to_string(sum));
  }
  return sum;
}

static void BM_TradingWorkNoLogging(benchmark::State& state) {
  const int operations = state.range(0);

  for (auto _ : state) {
    double result = doTradingWorkNoLogging(operations);
    benchmark::DoNotOptimize(result);
  }
  state.SetComplexityN(state.range(0));
}

static void BM_TradingWorkWithLogging(benchmark::State& state) {
  const int operations = state.range(0);

  for (auto _ : state) {
    double result = doTradingWorkWithLogging(operations);
    benchmark::DoNotOptimize(result);
  }
  state.SetComplexityN(state.range(0));
}

BENCHMARK(BM_TradingWorkNoLogging)->Range(1000,1000000);
BENCHMARK(BM_TradingWorkWithLogging)->Range(1000,1000000);

BENCHMARK_MAIN();