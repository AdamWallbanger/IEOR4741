//
// Created by tianci chen on 10/15/25.
//
#pragma once
#include "side_book.h"
#include <unordered_map>

#ifndef _BUILDING_A_HIGH_PERFORMANCE_C___ORDER_BOOK__ORDER_BOOK_H
#define _BUILDING_A_HIGH_PERFORMANCE_C___ORDER_BOOK__ORDER_BOOK_H

namespace lob {

    class OrderBook {
    public:
        explicit OrderBook(size_t reserve = 1'000'000);

        bool newOrder(const Order& o);
        bool amendOrder(id_t id, qty_t newQty);
        bool deleteOrder(id_t id);

        PriceLevel topOfBook(Side s) const;
        size_t orderCount(ticks_t price) const;
        qty_t totalVolume(ticks_t price) const;

    private:
        std::unordered_map<id_t, ticks_t> id2price;
        SideBook bids, asks;
    };

} // namespace lob


#endif //_BUILDING_A_HIGH_PERFORMANCE_C___ORDER_BOOK__ORDER_BOOK_H
