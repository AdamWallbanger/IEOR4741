//
// Created by tianci chen on 10/15/25.
//
#pragma once
#include "price_level.h"
#include <map>
#include <queue>
#include <vector>

#ifndef _BUILDING_A_HIGH_PERFORMANCE_C___ORDER_BOOK__SIDE_BOOK_H
#define _BUILDING_A_HIGH_PERFORMANCE_C___ORDER_BOOK__SIDE_BOOK_H

// side_book.h
#pragma once
#include "price_level.h"
#include <map>
#include <queue>
#include <vector>
#include <functional>

namespace lob {

    struct SideBook {
        std::map<ticks_t, PriceLevel> levels;

        // Max-heap for bids, Min-heap for asks
        mutable std::priority_queue<ticks_t> bidHeap;
        mutable std::priority_queue<ticks_t,
                std::vector<ticks_t>,
                std::greater<ticks_t>> askHeap;
    };

} // namespace lob


#endif //_BUILDING_A_HIGH_PERFORMANCE_C___ORDER_BOOK__SIDE_BOOK_H
