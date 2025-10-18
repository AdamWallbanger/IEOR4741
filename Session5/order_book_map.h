//
// Created by tianci chen on 10/16/25.
//

#ifndef HIGHPERFORDERBOOK_ORDER_BOOK_MAP_H
#define HIGHPERFORDERBOOK_ORDER_BOOK_MAP_H
#pragma once
#include "order.h"
#include "price_level.h"
#include "side_book.h"  // reuse SideBook's map but ignore its heaps or define your own simple struct
#include <unordered_map>
#include <map>

namespace lob {

    struct IdLocM { Side side; ticks_t price; qty_t qty; };

    class OrderBookMap {
    public:
        explicit OrderBookMap(size_t reserve = 1'000'000) {
            id2loc.reserve(reserve);
            id2loc.max_load_factor(0.5f);
            id2loc.rehash(static_cast<size_t>(reserve / 0.5f));
        }

        bool newOrder(const Order& o) {
            auto& sb  = (o.side == Side::Buy) ? bids : asks;
            auto& lvl = sb[o.price]; // creates if missing
            lvl.price = o.price;
            lvl.totalQty += o.qty;
            lvl.orderCount += 1;
            id2loc[o.id] = {o.side, o.price, o.qty};
            return true;
        }

        bool amendOrder(id_t id, qty_t newQty) {
            auto it = id2loc.find(id);
            if (it == id2loc.end() || newQty <= 0) return false;
            auto [side, px, oldQty] = it->second;
            auto& lvl = (side == Side::Buy ? bids[px] : asks[px]);
            lvl.totalQty += (newQty - oldQty);
            it->second.qty = newQty;
            return true;
        }

        bool deleteOrder(id_t id) {
            auto it = id2loc.find(id);
            if (it == id2loc.end()) return false;
            auto [side, px, oldQty] = it->second;
            auto& lvl = (side == Side::Buy ? bids[px] : asks[px]);
            lvl.orderCount -= 1;
            lvl.totalQty   -= oldQty;
            id2loc.erase(it);
            return true;
        }

        PriceLevel topOfBook(Side s) const {
            if (s == Side::Buy) {
                for (auto it = bids.rbegin(); it != bids.rend(); ++it)
                    if (it->second.orderCount > 0) return it->second;
            } else {
                for (auto it = asks.begin(); it != asks.end(); ++it)
                    if (it->second.orderCount > 0) return it->second;
            }
            return {0,0,0};
        }

        size_t orderCount(ticks_t price) const {
            size_t c=0;
            auto itB=bids.find(price); if(itB!=bids.end()) c += itB->second.orderCount;
            auto itA=asks.find(price); if(itA!=asks.end()) c += itA->second.orderCount;
            return c;
        }
        qty_t totalVolume(ticks_t price) const {
            qty_t v=0;
            auto itB=bids.find(price); if(itB!=bids.end()) v += itB->second.totalQty;
            auto itA=asks.find(price); if(itA!=asks.end()) v += itA->second.totalQty;
            return v;
        }

    private:
        std::unordered_map<id_t, IdLocM> id2loc;
        std::map<ticks_t, PriceLevel> bids; // rbegin() is best bid
        std::map<ticks_t, PriceLevel> asks; // begin()  is best ask
    };

} // namespace lob

#endif //HIGHPERFORDERBOOK_ORDER_BOOK_MAP_H
