#include <vector>
#include "order.h"

class OrderBook {
public:

    OrderBook() = default;

    // Returns true if order was executed
    // Returns false if order is added to the vector
    bool process_order(const Order& order); 

    // displays the current orderbook
    void display_order();

private:
    // Use two vectors to store buy and sell orders separately for easier matching and for cache locality
    // rather than using a queue
    // sorted order in opposite fashion as nature of vector and most volume at the center points of both
    // books
    std::vector<Order> bids_; // sorted order of buying orders, increasing
    std::vector<Order> asks_; // sorted order of selling orders, decreasing

};