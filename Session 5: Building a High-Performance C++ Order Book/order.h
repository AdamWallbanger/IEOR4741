//
// Created by tianci chen on 10/15/25.
//
#pragma once
#include <cstdint>

#ifndef _BUILDING_A_HIGH_PERFORMANCE_C___ORDER_BOOK__ORDER_H
#define _BUILDING_A_HIGH_PERFORMANCE_C___ORDER_BOOK__ORDER_H

namespace lob {

    using id_t    = uint64_t;
    using qty_t   = int32_t;
    using ticks_t = int32_t;

    enum class Side : uint8_t { Buy = 0, Sell = 1 };

    struct alignas(64) Order {
        id_t id;
        ticks_t price;
        qty_t qty;
        Side side;
    };

} // namespace lob

#endif //_BUILDING_A_HIGH_PERFORMANCE_C___ORDER_BOOK__ORDER_H
