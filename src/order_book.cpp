#include <algorithm>

#include "order_book.h"

OrderResult OrderBook::process_order(const Order& order) {

    Order::Side side = order.side();

    // We want to take the book that is opposite to the order side
    // buy looks at sellers (asks), sell looks at buyers (bids)
    auto& opposite_book = (side == Order::Side::BUY) ? asks_ : bids_;

    // buy rests at buyers (bids), sell rests at sellers (asks)
    auto& resting_book = (side == Order::Side::BUY) ? bids_ : asks_;

    // handles when there is a price_cents that is better than the order
    // if the buy exceeds the min seller price_cents, we automatically set the buy to the min ask, for example
    auto passes_best_price = (order.side() == Order::Side::BUY)
        ? [](uint64_t buyer_p, uint64_t seller_p){ return buyer_p >= seller_p; }
        : [](uint64_t seller_p, uint64_t buyer_p){ return seller_p <= buyer_p; };

    // handles ordering based on the resting book sorting order using bin search
    auto book_order_cmp = (order.side() == Order::Side::BUY)
        ? [](const Order& order, uint64_t price_cents) { return order.price_cents() < price_cents; }
        : [](const Order& order, uint64_t price_cents) { return order.price_cents() > price_cents; };

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
        uint32_t filled_amount = order.quantity() - remaining;
        if (opposite_book.empty()) {
            if (remaining != order.quantity()) {
                return OrderResult(OrderResult::OrderStatus::PartiallyFilled, filled_amount);
            }
            return OrderResult(OrderResult::OrderStatus::Cancelled);
        }
        return OrderResult(OrderResult::OrderStatus::Filled, filled_amount);
    } else { // LIMIT, todo: need to check if this shit needs to be checked for error
        uint32_t remaining = order.quantity();
        uint64_t price_cents = order.price_cents();

        while (remaining > 0 && !opposite_book.empty() && passes_best_price(price_cents, opposite_book.back().price_cents())) {
            auto& top = opposite_book.back();
            if (remaining < top.quantity()) {
                top.reduce_quantity(remaining);
                remaining = 0;
            } else {
                remaining -= top.quantity();
                opposite_book.pop_back();
            }
        }
        uint32_t filled_amount = order.quantity() - remaining;
        if (remaining > 0) {
            Order new_order = order;

            new_order.reduce_quantity(filled_amount);
            auto it = std::lower_bound(resting_book.begin(), resting_book.end(), price_cents, book_order_cmp);
            if (it == resting_book.end()) {
                resting_book.emplace_back(std::move(new_order));
                if (remaining == order.quantity()) return OrderResult(OrderResult::OrderStatus::Pending, filled_amount);
                return OrderResult(OrderResult::OrderStatus::PartiallyFilled, filled_amount);
            }
            resting_book.insert(it, std::move(new_order));
            if (remaining == order.quantity()) return OrderResult(OrderResult::OrderStatus::Pending, filled_amount);
            return OrderResult(OrderResult::OrderStatus::PartiallyFilled, filled_amount);
        } else {
            return OrderResult(OrderResult::OrderStatus::Filled, filled_amount);
        }
    }
}

uint32_t OrderBook::quanitity_at_price(double price, Order::Side side) {
    uint64_t price_cents = static_cast<uint64_t>(price * 100);
    auto& resting_book = (side == Order::Side::BUY) ? bids_ : asks_;

    // handles ordering based on the resting book sorting order using bin search
    auto book_order_cmp = (side == Order::Side::BUY)
        ? [](const Order& order, uint64_t price_cents) { return order.price_cents() < price_cents; }
        : [](const Order& order, uint64_t price_cents) { return order.price_cents() > price_cents; };
    
    auto it = std::lower_bound(resting_book.begin(), resting_book.end(), price_cents, book_order_cmp);

    uint32_t quantity = 0;

    while (it != resting_book.end() && it->price_cents() == price_cents) {
        quantity += it->quantity();
        ++it;
    }

    return quantity;
}

void OrderBook::display_order() {
    return;

}
