#include <algorithm>
#include <random>
#include <unordered_map>
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

template <typename Orderbook>
std::vector<uint64_t> benchmark_cancel(size_t N) {
    Orderbook book;
    
    std::vector<uint64_t> elapsed_times;
    elapsed_times.reserve(N);

    std::mt19937_64 rng(67);
    std::normal_distribution<double> bid_dist (99.5, 0.1);
    std::normal_distribution<double> ask_dist (100.5, 0.1);
    std::uniform_int_distribution<uint32_t> quantity_dist (1, 10000);

    std::vector<uint64_t> ids;
    ids.reserve(N);

    // Fill the books
    for(size_t i = 0; i < N; ++i) {
        Order::Side side = (i % 2 == 0) ? Order::Side::BUY : Order::Side::SELL;
        double price = (side == Order::Side::BUY) ? bid_dist(rng) : ask_dist(rng);
        Order o(price, quantity_dist(rng), Order::Type::LIMIT, side);
        OrderResult res = book.process_order(o);
        ids.push_back(res.order_id());
    }

    std::shuffle(ids.begin(), ids.end(), rng);

    // Cancel all the orders randomly
    for (auto id: ids) {
        LatencyTimer t;
        OrderResult res = book.cancel_order(id);
        t.stop();
        elapsed_times.push_back(t.elapsed_ns());
    }

    return elapsed_times;
}

template <typename Orderbook>
std::vector<uint64_t> benchmark_match(size_t N) {
    Orderbook book;
    
    std::vector<uint64_t> elapsed_times;
    elapsed_times.reserve(N);

    std::mt19937_64 rng(67);
    std::normal_distribution<double> bid_dist (99.5, 0.1);
    std::normal_distribution<double> ask_dist (100.5, 0.1);

    // Fixed, equal quantities so each aggressor fully consumes exactly one resting
    // order: a clean pop_back + id map erase, with no remainder left to rest.
    uint32_t fixedQuantity = 100;

    // Aggressor limits priced through the touch so they cross reliably:
    // a taker BUY reaches up above the resting asks, a taker SELL reaches down below the resting bids.
    std::normal_distribution<double> buy_aggressor_dist (100.7, 0.1);
    std::normal_distribution<double> sell_aggressor_dist (99.3, 0.1);

    // Rest 2N orders so firing N aggressors can never drain the book mid-run.
    for(size_t i = 0; i < 2 * N; ++i) {
        Order::Side side = (i % 2 == 0) ? Order::Side::BUY : Order::Side::SELL;
        double price = (side == Order::Side::BUY) ? bid_dist(rng) : ask_dist(rng);
        Order o(price, fixedQuantity, Order::Type::LIMIT, side);
        OrderResult res = book.process_order(o);
    }

    for(size_t i = 0; i < N; ++i) {
        Order::Side side = (i % 2 == 0) ? Order::Side::SELL : Order::Side::BUY;
        double price = (side == Order::Side::BUY) ? buy_aggressor_dist(rng) : sell_aggressor_dist(rng);
        Order o(price, fixedQuantity, Order::Type::LIMIT, side);

        LatencyTimer t;
        OrderResult res = book.process_order(o);
        t.stop();
        elapsed_times.push_back(t.elapsed_ns());
    }

    return elapsed_times;
}