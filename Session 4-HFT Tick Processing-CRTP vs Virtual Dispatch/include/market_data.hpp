#pragma once
#include <cstdint>

struct Quote {
    double bid;
    double ask;
    double bid_qty;
    double ask_qty;
};

// --- Pure math helpers (header-only, likely to inline) ---
// 使用倒数乘法（branchless tweak）
inline double mid(const Quote& q) noexcept {
    return (q.bid + q.ask) * 0.5;
}

inline double microprice(const Quote& q) noexcept {
    const double denom = q.bid_qty + q.ask_qty; // Assumption: > 0 in synthetic stream
    const double inv   = 1.0 / denom;
    return (q.bid * q.ask_qty + q.ask * q.bid_qty) * inv;
}

inline double imbalance(const Quote& q) noexcept {
    const double denom = q.bid_qty + q.ask_qty; // Assumption: > 0
    const double inv   = 1.0 / denom;
    return (q.bid_qty - q.ask_qty) * inv;
}
