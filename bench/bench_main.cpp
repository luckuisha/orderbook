#include <algorithm>
#include <format>
#include <iostream>
#include <numeric>
#include <string>

#include "order_book.h"
#include "work.h"

static void report(std::string benchmark_name, std::vector<uint64_t> elapsed) {

    std::sort(elapsed.begin(), elapsed.end());

    size_t count = elapsed.size();

    auto percentile = [&elapsed, count](double percentage) {
        size_t percentile_index = std::min(static_cast<size_t>(count * percentage), count - 1);
        return elapsed[percentile_index];
    };

    uint64_t total_time = std::accumulate(elapsed.begin(), elapsed.end(), uint64_t{0});
    uint64_t ops_per_second = count / (total_time / 1e9);

    std::cout << "Benchmark Report " << benchmark_name << "\n";
    std::cout << std::format("Total Samples: {}", count) << "\n";
    std::cout << std::format("Total Time: {} ns", total_time) << "\n";
    std::cout << std::format("Ops per second: {}", ops_per_second) << "\n";
    std::cout << std::format("Median: {} ns", percentile(0.5)) << "\n";
    std::cout << std::format("0.99: {} ns", percentile(0.99)) << "\n";
    std::cout << std::format("0.999: {} ns", percentile(0.999)) << "\n";
    std::cout << std::format("0.99999: {} ns", percentile(0.99999)) << "\n";
    std::cout << std::endl;
}

int main() {
    auto insert = benchmark_insert<OrderBook>(100'000);
    report("BENCH INSERTION", insert);

    auto cancel = benchmark_cancel<OrderBook>(100'000);
    report("BENCH CANCEL", cancel);

    auto match = benchmark_match<OrderBook>(100'000);
    report("BENCH MATCH", cancel);

    return 0;
}