#pragma once
#include <chrono>
#include <string>
struct alignas(64) MarketData {
    std::string symbol;
    double bid_price;
    double ask_price;
    std::chrono::high_resolution_clock::time_point timestamp;
};
MarketData random_marketdata(string symbol);