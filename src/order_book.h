#include <vector>

#include "order_result.h"
#include "order.h"

class OrderBook {
public:

    OrderBook() = default;

    // Returns true if order was executed
    // Returns false if order is added to the vector
    OrderResult process_order(const Order& order); 

    // Returns total quantity at a price_cents point
    uint32_t quanitity_at_price(double price, Order::Side side);

    // displays the current orderbook
    void display_order() const;

private:
    // Use two vectors to store buy and sell orders separately for easier matching and for cache locality
    // rather than using a queue
    // sorted order in opposite fashion as nature of vector and most volume at the center points of both
    // books
    std::vector<Order> bids_; // sorted order of buying orders, increasing
    std::vector<Order> asks_; // sorted order of selling orders, decreasing

};
