#include <cstdint>

class Order {
public:
    enum class Side {
        BUY, SELL,
    };

    enum class Type {
        MARKET, LIMIT, 
    };

    Order(double price, int quantity, Type type, Side side)
        : price_(static_cast<uint64_t>(price * 100)), quantity_(quantity), type_(type), side_(side) {}

    Order(uint64_t price, int quantity, Type type, Side side)
        : price_(price), quantity_(quantity), type_(type), side_(side) {}

    uint64_t price() const { return price_; }

    double human_price() const { return price_ / 100.0; }

    uint32_t quantity() const { return quantity_; }
    void quantity(uint32_t quantity) { quantity_ = quantity; }
    void reduce_quantity(uint32_t quantity) { quantity_ -= quantity; }

    Type type() const { return type_; }

    Side side() const { return side_; }

private:
    uint64_t price_; // Price are in cents to avoid floating point precision issues
    uint32_t quantity_;
    Type type_;
    Side side_;
};
