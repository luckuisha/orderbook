#include <iostream>

#include "order_book.h"
#include "order_result.h"

int main() {
    OrderBook ob = OrderBook();
    Order o = Order(100.12, 1, Order::Type::MARKET, Order::Side::SELL);

    OrderResult sold = ob.process_order(o);
    std::cout<< static_cast<int>(sold.status()) << std::endl;
    std::cin.get();

}