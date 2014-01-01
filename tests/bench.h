
#ifndef BOTAN_CHECK_BENCHMARK_H__
#define BOTAN_CHECK_BENCHMARK_H__

#include <botan/rng.h>
#include <botan/transform.h>
#include <string>

using namespace Botan;

void benchmark(double seconds,
               size_t buf_size);

void bench_algo(const std::string& algo_name,
                RandomNumberGenerator& rng,
                double seconds,
                size_t buf_size);

void bench_pk(RandomNumberGenerator& rng,
              const std::string& algo, double seconds);

#endif
