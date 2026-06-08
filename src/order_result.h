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
    OrderResult(OrderStatus status, uint32_t quantity): status_(status), filled_quantity_(quantity) {}
    OrderResult(OrderStatus status, uint32_t quantity, uint64_t id)
        : status_(status), filled_quantity_(quantity), order_id_(id) {}

    OrderStatus status() const { return status_; }

private:
    OrderStatus status_;
    uint32_t filled_quantity_ = 0;
    uint64_t order_id_ = 0; // 0 means not resting
};