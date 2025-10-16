//
// Created by tianci chen on 10/15/25.
//
#include "order_book.h"
#include "order.h"
#include "price_level.h"
#include "side_book.h"  // reuse SideBook's map but ignore its heaps or define your own simple struct
#include <unordered_map>
#include <map>
using namespace lob;

OrderBook::OrderBook(size_t reserve) {
    id2price.reserve(reserve);
}

bool OrderBook::newOrder(const Order& o) {
    auto& sb = (o.side == Side::Buy) ? bids : asks;
    auto& lvl = sb.levels[o.price];
    lvl.price = o.price;
    lvl.totalQty += o.qty;
    lvl.orderCount += 1;
    id2price[o.id] = o.price;
    if (o.side == Side::Buy)
        sb.bidHeap.push(o.price);
    else
        sb.askHeap.push(o.price);
    return true;
}

bool OrderBook::amendOrder(id_t id, qty_t newQty) {
    auto it = id2price.find(id);
    if (it == id2price.end()) return false;
    ticks_t px = it->second;

    auto* sb = (bids.levels.count(px)) ? &bids : &asks;
    auto& lvl = sb->levels[px];
    lvl.totalQty = newQty;  // simplified logic
    return true;
}

bool OrderBook::deleteOrder(id_t id) {
    auto it = id2price.find(id);
    if (it == id2price.end()) return false;
    ticks_t px = it->second;
    auto* sb = (bids.levels.count(px)) ? &bids : &asks;
    auto& lvl = sb->levels[px];
    lvl.orderCount -= 1;
    id2price.erase(it);
    return true;
}

// order_book.cpp
PriceLevel OrderBook::topOfBook(Side s) const {
    if (s == Side::Buy) {
        // lazily purge stale prices from the actual heap
        while (!bids.bidHeap.empty()) {
            ticks_t p = bids.bidHeap.top();
            auto it = bids.levels.find(p);
            if (it != bids.levels.end() && it->second.orderCount > 0) {
                return it->second;
            }
            bids.bidHeap.pop(); // stale
        }
        // fallback: walk map from back (highest price)
        for (auto it = bids.levels.rbegin(); it != bids.levels.rend(); ++it) {
            if (it->second.orderCount > 0) return it->second;
        }
        return {0, 0, 0};
    } else {
        while (!asks.askHeap.empty()) {
            ticks_t p = asks.askHeap.top();
            auto it = asks.levels.find(p);
            if (it != asks.levels.end() && it->second.orderCount > 0) {
                return it->second;
            }
            asks.askHeap.pop(); // stale
        }
        // fallback: walk map from front (lowest price)
        for (auto it = asks.levels.begin(); it != asks.levels.end(); ++it) {
            if (it->second.orderCount > 0) return it->second;
        }
        return {0, 0, 0};
    }
}

size_t OrderBook::orderCount(ticks_t price) const {
    size_t c = 0;
    auto itB = bids.levels.find(price);
    if (itB != bids.levels.end()) c += itB->second.orderCount;
    auto itA = asks.levels.find(price);
    if (itA != asks.levels.end()) c += itA->second.orderCount;
    return c;
}

qty_t OrderBook::totalVolume(ticks_t price) const {
    qty_t v = 0;
    auto itB = bids.levels.find(price);
    if (itB != bids.levels.end()) v += itB->second.totalQty;
    auto itA = asks.levels.find(price);
    if (itA != asks.levels.end()) v += itA->second.totalQty;
    return v;
}
