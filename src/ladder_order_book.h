#include <unordered_map>
#include <vector>

#include "order_result.h"
#include "order.h"

class LadderOrderBook {
public:

    LadderOrderBook() = default;

    // Returns true if order was executed
    // Returns false if order is added to the vector
    OrderResult process_order(const Order& order); 

    OrderResult cancel_order(uint64_t id);

    OrderResult modify_order(const uint64_t id, double new_price, uint32_t new_quantity);

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

    uint64_t next_id_ = 1; // 0 means unassigned, keeps incrementing upon order creation
    std::unordered_map<uint64_t, Order::Side> id_lookup_side_; 

};
