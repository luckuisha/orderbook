#include <iostream>
#include "order_book.h"

int main() {
    OrderBook ob = OrderBook();
    Order o = Order(100.12, 1, Order::Type::MARKET, Order::Side::SELL);

    bool sold = ob.process_order(o);
    std::cout<<sold << std::endl;
    std::cin.get();

}