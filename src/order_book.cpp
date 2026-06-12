#include <algorithm>
#include <format>
#include <iostream>
#include <ranges>
#include <string>

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
            uint64_t id = top.id();
            if (remaining < top.quantity()) {
                top.reduce_quantity(remaining);
                remaining = 0;
            } else {
                remaining -= top.quantity();
                opposite_book.pop_back();
                id_lookup_side_.erase(id);
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
            uint64_t id = top.id();
            if (remaining < top.quantity()) {
                top.reduce_quantity(remaining);
                remaining = 0;
            } else {
                remaining -= top.quantity();
                opposite_book.pop_back();
                id_lookup_side_.erase(id);
            }
        }
        uint32_t filled_amount = order.quantity() - remaining;
        if (remaining > 0) {
            Order new_order = order;
            new_order.reduce_quantity(filled_amount);
            uint64_t id = next_id_++;
            id_lookup_side_[id] = new_order.side();
            new_order.id(id);

            auto it = std::lower_bound(resting_book.begin(), resting_book.end(), price_cents, book_order_cmp);
            resting_book.insert(it, std::move(new_order));

            auto status = (remaining == order.quantity()) ? OrderResult::OrderStatus::Pending : OrderResult::OrderStatus::PartiallyFilled;
            return OrderResult(status, filled_amount, id);
        } else {
            return OrderResult(OrderResult::OrderStatus::Filled, filled_amount);
        }
    }
}

OrderResult OrderBook::cancel_order(uint64_t id) {
    auto it = id_lookup_side_.find(id);
    if (it == id_lookup_side_.end()) return OrderResult(OrderResult::OrderStatus::Rejected);

    auto& book = (it->second == Order::Side::BUY) ? bids_ : asks_;
    auto found = std::find_if(book.begin(), book.end(), [id](const Order& o){ return o.id() == id; });
    if (found == book.end()) return OrderResult(OrderResult::OrderStatus::Rejected);
    book.erase(found);
    id_lookup_side_.erase(id);
    return OrderResult(OrderResult::OrderStatus::Cancelled);
}

OrderResult OrderBook::modify_order(uint64_t id, double new_price, uint32_t new_quantity) {
    auto it = id_lookup_side_.find(id);
    if (it == id_lookup_side_.end()) return OrderResult(OrderResult::OrderStatus::Rejected);

    auto& book = (it->second == Order::Side::BUY) ? bids_ : asks_;
    auto found = std::find_if(book.begin(), book.end(), [id](const Order& o){ return o.id() == id; });
    if (found == book.end()) return OrderResult(OrderResult::OrderStatus::Rejected);

    uint64_t price_cents = static_cast<uint64_t>(new_price * 100);
    if (found->price_cents() == price_cents && found->quantity() == new_quantity) {
        return OrderResult(OrderResult::OrderStatus::Rejected);
    }

    if (found->price_cents() == price_cents && new_quantity < found->quantity()) {
        found->quantity(new_quantity);
        return OrderResult(OrderResult::OrderStatus::Pending, new_quantity);
    }

    book.erase(found);
    id_lookup_side_.erase(it);

    Order replacemant(price_cents, new_quantity, found->type(), found->side());
    return process_order(replacemant);
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

void OrderBook::display_order() const {

    std::string symbol = "█";

    // start with buys and end with sells, both need to be iterated backwards
    if (!bids_.empty()){
        size_t i = 0;
        while (i < bids_.size()) {
            const uint64_t level_price = bids_[i].price_cents();
            uint32_t level_quantity = 0;

            while (i < bids_.size() && bids_[i].price_cents() == level_price) {
                level_quantity += bids_[i].quantity();
                ++i;
            } 

            std::cout << std::format("{:>10.2f} : ", level_price / 100.0);
            for (uint32_t j = 0; j < (level_quantity + 9) / 10; ++j) {
                std::cout << symbol;
            }
            std::cout << "\n";
        }
    }

    if (!bids_.empty() && !asks_.empty()) {
        uint64_t spread = asks_.back().price_cents() - bids_.back().price_cents();
        std::cout << std::format("Spread is {:.2f}\n", spread / 100.0);
    }

    if (!asks_.empty()){
        int i = asks_.size() - 1;
        while (i >= 0) {
            const uint64_t level_price = asks_[i].price_cents();
            uint32_t level_quantity = 0;

            while (i >= 0 && asks_[i].price_cents() == level_price) {
                level_quantity += asks_[i].quantity();
                --i;
            } 

            std::cout << std::format("{:>10.2f} : ", level_price / 100.0);
            for (uint32_t j = 0; j < (level_quantity + 9) / 10; ++j) {
                std::cout << symbol;
            }
            std::cout << "\n";
        }
    }
    std::cout << std::endl;
    return;
}
