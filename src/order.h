#pragma once

#include <cstdint>

class Order {
public:
    enum class Side {
        BUY, SELL,
    };

    enum class Type {
        MARKET, LIMIT, 
    };

    Order(double price_cents, int quantity, Type type, Side side)
        : price_(static_cast<uint64_t>(price_cents * 100)), quantity_(quantity), type_(type), side_(side) {}

    void id(uint64_t id) { id_ = id; }

    uint64_t id() const { return id_; }

    uint64_t price_cents() const { return price_; }

    double price() const { return price_ / 100.0; }

    uint32_t quantity() const { return quantity_; }
    void quantity(uint32_t quantity) { quantity_ = quantity; }
    void reduce_quantity(uint32_t quantity) { quantity_ -= quantity; }
    void increase_quantity(uint32_t quantity) { quantity_ += quantity; }

    Type type() const { return type_; }

    Side side() const { return side_; }

private:
    uint64_t id_ = 0; // 0 is unassigned
    uint64_t price_; // Price are in cents to avoid floating point precision issues
    uint32_t quantity_;
    Type type_;
    Side side_;
};
