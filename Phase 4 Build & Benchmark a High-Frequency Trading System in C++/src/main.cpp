#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>

#include "../include/MarketData.hpp"
#include "../include/Order.hpp"
#include "../include/Timer.hpp"
#include "../include/OrderManager.hpp"
#include "../include/OrderBook.hpp"
#include "../include/MatchingEngine.hpp"
using namespace std;

using OrderType = Order<double, int>;

int main() {
    random_device rd;
    mt19937 seed(rd());
    std::vector<long long> latencies;
    const int num_ticks = 10000;
    MarketData market_data =  random_marketdata("AAPL");
    OrderStore<double,int> book;
    OrderManagement<double,int> oms(book);
    for (int i = 0; i < num_ticks; ++i)
    {
        uniform_real_distribution<double> price(market_data.bid_prices-10*0.01, market_date.ask_prices+10*0.01);

        Timer timer;
        timer.start();
        OrderType order(i, "AAPL", price(seed), 100, i % 2 == 0);
        auto* o = oms.add(order.id, order.price, order.quantity, order.is_buy);
        auto t0 = chrono::high_resolution_clock::now();
        auto trades = match_after_add(book, oms,o->id, t0);
        latencies.push_back(timer.stop());
    }

    // Analyze latency
    auto min = *std::min_element(latencies.begin(), latencies.end());
    auto max = *std::max_element(latencies.begin(), latencies.end());
    double mean = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();

    std::cout << "Tick-to-Trade Latency (nanoseconds):\n";
    std::cout << "Min: " << min << " | Max: " << max << " | Mean: " << mean << '\n';
}