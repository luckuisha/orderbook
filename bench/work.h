#include <random>
#include <vector>

#include "latency_timer.h"
#include "order.h"


template <typename Orderbook>
std::vector<uint64_t> benchmark_insert(size_t N) {
    Orderbook book;
    std::vector<uint64_t> elapsed_times;
    elapsed_times.reserve(N);

    std::mt19937_64 rng(67);
    std::normal_distribution<double> bid_dist (99.5, 0.1);
    std::normal_distribution<double> ask_dist (100.5, 0.1);
    std::uniform_int_distribution<uint32_t> quantity_dist (1, 10000);

    for(size_t i = 0; i < N; ++i) {
        Order::Side side = (i % 2 == 0) ? Order::Side::BUY : Order::Side::SELL;
        double price = (side == Order::Side::BUY) ? bid_dist(rng) : ask_dist(rng);

        Order o(price, quantity_dist(rng), Order::Type::LIMIT, side);

        LatencyTimer t;
        book.process_order(o);
        t.stop();
        elapsed_times.push_back(t.elapsed_ns());
    }
    return elapsed_times;
}
