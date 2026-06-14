#pragma once

#include <chrono>
#include <cstdint>

class LatencyTimer {

public:
    LatencyTimer() : start_(std::chrono::steady_clock::now()) {};

    void stop() {
        if (stopped_) return;
        auto end = std::chrono::steady_clock::now();
        elapsed_ns_ = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_).count();
        stopped_ = true;
    }
    
    uint64_t elapsed_ns() const {
        return elapsed_ns_;
    }

private:
    std::chrono::time_point<std::chrono::steady_clock> start_;
    uint64_t elapsed_ns_ = 0;
    bool stopped_ = false;
};