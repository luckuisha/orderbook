#include <iostream>

#include "order_book.h"
#include "order_result.h"

int main() {
    OrderBook ob = OrderBook();
    Order o = Order(100.12, 1, Order::Type::MARKET, Order::Side::SELL);

    OrderResult sold = ob.process_order(o);
    std::cout<< static_cast<int>(sold.status()) << std::endl;
    ob.process_order(Order(100.00, 50, Order::Type::LIMIT, Order::Side::BUY));
    ob.process_order(Order(99.50, 30, Order::Type::LIMIT, Order::Side::BUY));
    ob.process_order(Order(101.00, 40, Order::Type::LIMIT, Order::Side::SELL));
    ob.process_order(Order(102.25, 20, Order::Type::LIMIT, Order::Side::SELL));
    ob.display_order();
    std::cin.get();

}