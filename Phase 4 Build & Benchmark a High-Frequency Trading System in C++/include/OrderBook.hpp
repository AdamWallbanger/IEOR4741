#pragma once
#include <map>
#include <memory>
#include <vector>
using namespace std;

template <typename PriceType, typename OrderIdType>
struct Order {
    OrderIdType id;
    PriceType price;
    int quantity;
    bool is_buy;
};

template <typename PriceType, typename OrderIdType>
struct OrderStore {
    using Ord = Order<PriceType, OrderIdType>;
    vector<unique_ptr<Ord>> owned;
    multimap<PriceType, Ord*> buys;
    multimap<PriceType, Ord*> sells;
};