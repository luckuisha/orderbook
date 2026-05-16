#include "order_book.h"

bool OrderBook::process_order(const Order& order) {
    // We want to take the book that is opposite to the order side
    // buy looks at sellers (asks), sell looks at buyers (bids)
    auto& opposite_book = (order.side() == Order::Side::BUY) ? asks_ : bids_;

    // buy rests at buyers (bids), sell rests at sellers (asks)
    auto& resting_book = (order.side() == Order::Side::BUY) ? bids_ : asks_;

    if (order.type() == Order::Type::MARKET) {
        uint32_t remaining = order.quantity();
        while (remaining > 0 && !opposite_book.empty()) {
            auto& top = opposite_book.back();
            if (remaining < top.quantity()) {
                top.reduce_quantity(remaining);
                remaining = 0;
            } else {
                remaining -= top.quantity();
                opposite_book.pop_back();
            }
        }
        return true;
    } else { // LIMIT, todo: need to check if this shit needs to be checked for error

    }
    return false;

}