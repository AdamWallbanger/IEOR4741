#pragma once
#include <vector>
#include <chrono>
using namespace std;

template <typename PriceType, typename OrderIdType>
struct Trade {
    OrderIdType buy_id;
    OrderIdType sell_id;
    PriceType   price;      // execute at best ask
    int         qty;
    long long   latency_ns; // -1 if not for the new order
};

template <typename PriceType, typename OrderIdType>
vector<Trade<PriceType, OrderIdType>>
match_after_add(
    OrderStore<PriceType, OrderIdType>& book,
    OrderManagement<PriceType, OrderIdType>& oms,
    OrderIdType new_order_id,
    chrono::high_resolution_clock::time_point arrival_ts
);