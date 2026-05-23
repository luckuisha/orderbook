#pragma once

class OrderResult {
public:
    enum class OrderStatus {
        Filled, 
        PartiallyFilled,
        Pending,
        Cancelled,
        Rejected
    };

    explicit OrderResult(OrderStatus status): status_(status) {}
    explicit OrderResult(OrderStatus status, uint32_t quantity): status_(status), filled_quantity_(quantity) {}

    OrderStatus status() const { return status_; }

private:
    OrderStatus status_;
    uint32_t filled_quantity_ = 0;
};