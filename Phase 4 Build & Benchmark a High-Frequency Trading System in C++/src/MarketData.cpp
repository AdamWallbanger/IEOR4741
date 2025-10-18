#include "../include/MarketData.hpp"
#include <random>
#include <chrono>
#include <string>
using namespace std;

MarketData random_marketdata(string symbol)
{
    random_device rd;
    mt19937 seed(rd());
    uniform_real_distribution<double> price(200, 300.0);
    uniform_real_distribution<double> spread(0.01, 1.00);

    double bid = price(seed);
    double ask = bid + spread(seed);

    return MarketData{symbol,bid,ask,chrono::high_resolution_clock::now()};
}
