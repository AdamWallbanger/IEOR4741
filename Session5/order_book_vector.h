//
// Created by tianci chen on 10/16/25.
//

#ifndef HIGHPERFORDERBOOK_ORDER_BOOK_VECTOR_H
#define HIGHPERFORDERBOOK_ORDER_BOOK_VECTOR_H
#pragma once
#include "order.h"
#include "price_level.h"
#include <unordered_map>
#include <vector>

namespace lob {

    struct IdLocV { Side side; ticks_t price; qty_t qty; };

    struct LevelVec {
        ticks_t price{0};
        qty_t   total{0};
        uint32_t count{0};
    };

    class OrderBookVector {
    public:
        explicit OrderBookVector(size_t reserve_ids = 1'000'000) {
            id2loc.reserve(reserve_ids);
            id2loc.max_load_factor(0.5f);
            id2loc.rehash(static_cast<size_t>(reserve_ids / 0.5f));
            bids.reserve(256);
            asks.reserve(256);
        }

        bool newOrder(const Order& o) {
            auto& side = (o.side == Side::Buy)? bids : asks;
            int ix = findLevel(side, o.price);
            if (ix < 0) { side.push_back(LevelVec{o.price,0,0}); ix = (int)side.size()-1; }
            side[ix].total += o.qty;
            side[ix].count += 1;
            id2loc[o.id] = {o.side, o.price, o.qty};
            return true;
        }

        bool amendOrder(id_t id, qty_t newQty) {
            auto it = id2loc.find(id);
            if (it == id2loc.end() || newQty <= 0) return false;
            auto [sideS, px, oldQty] = it->second;
            auto& side = (sideS == Side::Buy)? bids : asks;
            int ix = findLevel(side, px);
            if (ix < 0) return false;
            side[ix].total += (newQty - oldQty);
            it->second.qty = newQty;
            return true;
        }

        bool deleteOrder(id_t id) {
            auto it = id2loc.find(id);
            if (it == id2loc.end()) return false;
            auto [sideS, px, oldQty] = it->second;
            auto& side = (sideS == Side::Buy)? bids : asks;
            int ix = findLevel(side, px);
            if (ix < 0) return false;
            side[ix].count -= 1;
            side[ix].total -= oldQty;
            id2loc.erase(it);
            return true;
        }

        PriceLevel topOfBook(Side s) const {
            const auto& side = (s == Side::Buy)? bids : asks;
            if (side.empty()) return {0,0,0};
            int best = -1; ticks_t best_px = 0;
            for (int i=0;i<(int)side.size();++i) {
                if (side[i].count == 0) continue;
                if (best == -1) { best = i; best_px = side[i].price; continue; }
                if ( (s==Side::Buy && side[i].price > best_px) ||
                     (s==Side::Sell && side[i].price < best_px) ) {
                    best = i; best_px = side[i].price;
                }
            }
            if (best < 0) return {0,0,0};
            return PriceLevel{ side[best].price, side[best].total, side[best].count };
        }

        size_t orderCount(ticks_t price) const {
            size_t c=0;
            int ix = findLevel(bids, price); if (ix>=0) c += bids[ix].count;
            ix     = findLevel(asks, price); if (ix>=0) c += asks[ix].count;
            return c;
        }
        qty_t totalVolume(ticks_t price) const {
            qty_t v=0;
            int ix = findLevel(bids, price); if (ix>=0) v += bids[ix].total;
            ix     = findLevel(asks, price); if (ix>=0) v += asks[ix].total;
            return v;
        }

    private:
        static int findLevel(const std::vector<LevelVec>& side, ticks_t px) {
            for (int i=0;i<(int)side.size();++i) if (side[i].price == px) return i;
            return -1;
        }
        static int findLevel(std::vector<LevelVec>& side, ticks_t px) {
            for (int i=0;i<(int)side.size();++i) if (side[i].price == px) return i;
            return -1;
        }

        std::unordered_map<id_t, IdLocV> id2loc;
        std::vector<LevelVec> bids, asks;
    };

} // namespace lob

#endif //HIGHPERFORDERBOOK_ORDER_BOOK_VECTOR_H
