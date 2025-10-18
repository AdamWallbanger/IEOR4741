//
// Created by tianci chen on 10/15/25.
//
#pragma once
#include "order.h"

#ifndef _BUILDING_A_HIGH_PERFORMANCE_C___ORDER_BOOK__PRICE_LEVEL_H
#define _BUILDING_A_HIGH_PERFORMANCE_C___ORDER_BOOK__PRICE_LEVEL_H

namespace lob {

    struct alignas(64) PriceLevel {
        ticks_t price = 0;
        qty_t totalQty = 0;
        uint32_t orderCount = 0;
    };

} // namespace lob


#endif //_BUILDING_A_HIGH_PERFORMANCE_C___ORDER_BOOK__PRICE_LEVEL_H
