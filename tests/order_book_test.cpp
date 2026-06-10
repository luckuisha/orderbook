#include "doctest.h"
#include "order_book.h"

TEST_CASE("Testing the orderbook") {
    OrderBook book;

    // Test for `Cancelled` order results
    SUBCASE ("A market buy order when the book is empty") {
        Order buy_order(100.50, 100, Order::Type::MARKET, Order::Side::BUY);
        OrderResult res = book.process_order(buy_order);
        CHECK(res.status() == OrderResult::OrderStatus::Cancelled);
    }

    SUBCASE ("A market sell order when the book is empty") {
        Order sell_order(100.50, 100, Order::Type::MARKET, Order::Side::SELL);
        OrderResult res = book.process_order(sell_order);
        CHECK(res.status() == OrderResult::OrderStatus::Cancelled);
    }

    // Test for `Pending`
    SUBCASE("A limit sell order is added to the book when no match exists") {
        Order buy_order(90, 100, Order::Type::LIMIT, Order::Side::BUY);
        OrderResult res = book.process_order(buy_order);
        CHECK(res.status() == OrderResult::OrderStatus::Pending);
    }

    SUBCASE("A limit buy order is added to the book when no match exists") {
        Order sell_order(110, 100, Order::Type::LIMIT, Order::Side::SELL);
        OrderResult res = book.process_order(sell_order);
        CHECK(res.status() == OrderResult::OrderStatus::Pending);
    }

    // Test for `Filled` on perfect sized order
    SUBCASE("A sell order matches a perfect existing buy order") {
        Order buy_order(90, 5, Order::Type::LIMIT, Order::Side::BUY);
        OrderResult res = book.process_order(buy_order);
        CHECK(res.status() == OrderResult::OrderStatus::Pending);

        Order sell_order(90, 5, Order::Type::LIMIT, Order::Side::SELL);
        res = book.process_order(sell_order);
        CHECK(res.status() == OrderResult::OrderStatus::Filled);

        CHECK(book.quanitity_at_price(90, Order::Side::BUY) == 0);
    }

    SUBCASE("A buy order matches a perfect existing sell order") {
        Order sell_order(90, 5, Order::Type::LIMIT, Order::Side::SELL);
        OrderResult res = book.process_order(sell_order);
        CHECK(res.status() == OrderResult::OrderStatus::Pending);

        Order buy_order(90, 5, Order::Type::LIMIT, Order::Side::BUY);
        res = book.process_order(buy_order);
        CHECK(res.status() == OrderResult::OrderStatus::Filled);

        CHECK(book.quanitity_at_price(90, Order::Side::SELL) == 0);
    }

    // Test for `Filled` on smaller order
    SUBCASE("A small sell order matches an existing buy order") {
        Order buy_order(90, 5, Order::Type::LIMIT, Order::Side::BUY);
        OrderResult res = book.process_order(buy_order);
        CHECK(res.status() == OrderResult::OrderStatus::Pending);

        Order sell_order(90, 3, Order::Type::LIMIT, Order::Side::SELL);
        res = book.process_order(sell_order);
        CHECK(res.status() == OrderResult::OrderStatus::Filled);

        CHECK(book.quanitity_at_price(90, Order::Side::BUY) == 2);
    }

    SUBCASE("A small buy order matches an existing sell order") {
        Order sell_order(90, 5, Order::Type::LIMIT, Order::Side::SELL);
        OrderResult res = book.process_order(sell_order);
        CHECK(res.status() == OrderResult::OrderStatus::Pending);

        Order buy_order(90, 4, Order::Type::LIMIT, Order::Side::BUY);
        res = book.process_order(buy_order);
        CHECK(res.status() == OrderResult::OrderStatus::Filled);

        CHECK(book.quanitity_at_price(90, Order::Side::SELL) == 1);
    }

    // Test for `Filled` on multiple orders, FIFO
    SUBCASE("A sell order matches multiple existing buy orders") {
        Order buy_order(90, 5, Order::Type::LIMIT, Order::Side::BUY);
        OrderResult res = book.process_order(buy_order);
        CHECK(res.status() == OrderResult::OrderStatus::Pending);

        Order buy_order2(90, 5, Order::Type::LIMIT, Order::Side::BUY);
        res = book.process_order(buy_order2);
        CHECK(res.status() == OrderResult::OrderStatus::Pending);

        Order sell_order(90, 6, Order::Type::LIMIT, Order::Side::SELL);
        res = book.process_order(sell_order);
        CHECK(res.status() == OrderResult::OrderStatus::Filled);

        CHECK(book.quanitity_at_price(90, Order::Side::BUY) == 4);
    }

    SUBCASE("A buy order matches multiple existing sell orders") {
        Order sell_order(90, 5, Order::Type::LIMIT, Order::Side::SELL);
        OrderResult res = book.process_order(sell_order);
        CHECK(res.status() == OrderResult::OrderStatus::Pending);

        Order sell_order2(90, 5, Order::Type::LIMIT, Order::Side::SELL);
        res = book.process_order(sell_order2);
        CHECK(res.status() == OrderResult::OrderStatus::Pending);

        Order buy_order(90, 7, Order::Type::LIMIT, Order::Side::BUY);
        res = book.process_order(buy_order);
        CHECK(res.status() == OrderResult::OrderStatus::Filled);

        CHECK(book.quanitity_at_price(90, Order::Side::SELL) == 3);
    }

    // Test for `Partially Filled`
    SUBCASE("A large sell order matches an existing buy order") {
        Order buy_order(90, 3, Order::Type::LIMIT, Order::Side::BUY);
        OrderResult res = book.process_order(buy_order);
        CHECK(res.status() == OrderResult::OrderStatus::Pending);

        Order sell_order(90, 7, Order::Type::LIMIT, Order::Side::SELL);
        res = book.process_order(sell_order);
        CHECK(res.status() == OrderResult::OrderStatus::PartiallyFilled);

        CHECK(book.quanitity_at_price(90, Order::Side::SELL) == 4);
    }

    SUBCASE("A large buy order matches an existing sell order") {
        Order sell_order(90, 3, Order::Type::LIMIT, Order::Side::SELL);
        OrderResult res = book.process_order(sell_order);
        CHECK(res.status() == OrderResult::OrderStatus::Pending);

        Order buy_order(90, 10, Order::Type::LIMIT, Order::Side::BUY);
        res = book.process_order(buy_order);
        CHECK(res.status() == OrderResult::OrderStatus::PartiallyFilled);

        CHECK(book.quanitity_at_price(90, Order::Side::BUY) == 7);
    }

    // Test cancelling an order
    SUBCASE("Cancel a resting buy order removes its quantity from the book") {
        Order buy_order(90, 5, Order::Type::LIMIT, Order::Side::BUY);
        OrderResult res = book.process_order(buy_order);
        CHECK(res.status() == OrderResult::OrderStatus::Pending);
        CHECK(res.order_id() != 0);
        CHECK(book.quanitity_at_price(90, Order::Side::BUY) == 5);

        res = book.cancel_order(res.order_id());
        CHECK(res.status() == OrderResult::OrderStatus::Cancelled);
        CHECK(book.quanitity_at_price(90, Order::Side::BUY) == 0);
    }

    SUBCASE("Cancel a resting sell order removes its quantity from the book") {
        Order sell_order(110, 5, Order::Type::LIMIT, Order::Side::SELL);
        OrderResult res = book.process_order(sell_order);
        CHECK(res.status() == OrderResult::OrderStatus::Pending);
        CHECK(res.order_id() != 0);
        CHECK(book.quanitity_at_price(110, Order::Side::SELL) == 5);

        res = book.cancel_order(res.order_id());
        CHECK(res.status() == OrderResult::OrderStatus::Cancelled);
        CHECK(book.quanitity_at_price(110, Order::Side::SELL) == 0);
    }

    SUBCASE("Cancel with an unknown id is rejected") {
        OrderResult res = book.cancel_order(999);
        CHECK(res.status() == OrderResult::OrderStatus::Rejected);
    }

    SUBCASE("Double-cancel returns Rejected the second time") {
        Order buy_order(90, 5, Order::Type::LIMIT, Order::Side::BUY);
        OrderResult res = book.process_order(buy_order);
        uint64_t id = res.order_id();

        res = book.cancel_order(id);
        CHECK(res.status() == OrderResult::OrderStatus::Cancelled);

        res = book.cancel_order(id);
        CHECK(res.status() == OrderResult::OrderStatus::Rejected);
    }

    SUBCASE("Cancel after a full fill is rejected") {
        Order buy_order(90, 5, Order::Type::LIMIT, Order::Side::BUY);
        OrderResult res = book.process_order(buy_order);
        uint64_t id = res.order_id();

        Order sell_order(90, 5, Order::Type::LIMIT, Order::Side::SELL);
        res = book.process_order(sell_order);
        CHECK(res.status() == OrderResult::OrderStatus::Filled);

        res = book.cancel_order(id);
        CHECK(res.status() == OrderResult::OrderStatus::Rejected);
    }

    SUBCASE("Cancel only removes the targeted order at a price level") {
        Order o1(90, 5, Order::Type::LIMIT, Order::Side::BUY);
        OrderResult r1 = book.process_order(o1);
        Order o2(90, 3, Order::Type::LIMIT, Order::Side::BUY);
        OrderResult r2 = book.process_order(o2);
        CHECK(book.quanitity_at_price(90, Order::Side::BUY) == 8);

        OrderResult res = book.cancel_order(r1.order_id());
        CHECK(res.status() == OrderResult::OrderStatus::Cancelled);
        CHECK(book.quanitity_at_price(90, Order::Side::BUY) == 3);

        res = book.cancel_order(r2.order_id());
        CHECK(res.status() == OrderResult::OrderStatus::Cancelled);
        CHECK(book.quanitity_at_price(90, Order::Side::BUY) == 0);
    }

    SUBCASE("Cancel order when it has been partially filled") {
        // Place a resting buy
        Order buy_order(90, 10, Order::Type::LIMIT, Order::Side::BUY);
        OrderResult res = book.process_order(buy_order);
        CHECK(res.status() == OrderResult::OrderStatus::Pending);
        uint64_t id = res.order_id();
        CHECK(book.quanitity_at_price(90, Order::Side::BUY) == 10);

        // Sell into it for 4 — leaves 6 resting under the same ID
        Order sell_order(90, 4, Order::Type::LIMIT, Order::Side::SELL);
        res = book.process_order(sell_order);
        CHECK(res.status() == OrderResult::OrderStatus::Filled);
        CHECK(book.quanitity_at_price(90, Order::Side::BUY) == 6);

        // Cancel the original buy — the remaining 6 should be removed
        res = book.cancel_order(id);
        CHECK(res.status() == OrderResult::OrderStatus::Cancelled);
        CHECK(book.quanitity_at_price(90, Order::Side::BUY) == 0);
    }
}
