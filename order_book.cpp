#include <algorithm>

#include "order_book.h"


OrderResult OrderBook::process_order(const Order& order) {

    Order::Side side = order.side();

    // We want to take the book that is opposite to the order side
    // buy looks at sellers (asks), sell looks at buyers (bids)
    auto& opposite_book = (side == Order::Side::BUY) ? asks_ : bids_;

    // buy rests at buyers (bids), sell rests at sellers (asks)
    auto& resting_book = (side == Order::Side::BUY) ? bids_ : asks_;

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
        uint32_t price = order.price();

        if (side == Order::Side::BUY) {
            // handles when there is a price that is better than the order
            while (remaining > 0 && !opposite_book.empty() && price >= opposite_book.back().price()) {
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
                auto it = std::lower_bound(resting_book.begin(), resting_book.end(), price,
                    [](const Order& ord, uint32_t pr) {
                         return ord.price() < pr; 
                      }
                );
                if (it == resting_book.end()) {
                    resting_book.emplace_back(std::move(new_order));
                    if (remaining == order.quantity()) return OrderResult(OrderResult::OrderStatus::Pending, filled_amount);
                    return OrderResult(OrderResult::OrderStatus::PartiallyFilled, filled_amount);
                }
                resting_book.insert(it, std::move(new_order));
                if (remaining == order.quantity()) return OrderResult(OrderResult::OrderStatus::Pending, filled_amount);
                return OrderResult(OrderResult::OrderStatus::PartiallyFilled, filled_amount);
            } else if (remaining == 0) {
                return OrderResult(OrderResult::OrderStatus::Filled, filled_amount);
            }
            return OrderResult(OrderResult::OrderStatus::Rejected);
        } else { // SELL
            while (remaining > 0 && !opposite_book.empty() && price <= opposite_book.back().price()) {
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
                auto it = std::lower_bound(resting_book.begin(), resting_book.end(), price,
                    [](const Order& ord, uint32_t pr) {
                         return ord.price() > pr; 
                      }
                );
                if (it == resting_book.end()) {
                    resting_book.emplace_back(std::move(new_order));
                    if (remaining == order.quantity()) return OrderResult(OrderResult::OrderStatus::Pending, filled_amount);
                    return OrderResult(OrderResult::OrderStatus::PartiallyFilled, filled_amount);
                }
                resting_book.insert(it, std::move(new_order));
                if (remaining == order.quantity()) return OrderResult(OrderResult::OrderStatus::Pending, filled_amount);
                return OrderResult(OrderResult::OrderStatus::PartiallyFilled, filled_amount);
            } else if (remaining == 0) {
                return OrderResult(OrderResult::OrderStatus::Filled, filled_amount);
            }
            return OrderResult(OrderResult::OrderStatus::Rejected);
        }
        return OrderResult(OrderResult::OrderStatus::Rejected);
    }
    return OrderResult(OrderResult::OrderStatus::Rejected);
}