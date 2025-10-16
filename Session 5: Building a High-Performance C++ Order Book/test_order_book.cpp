//
// Created by tianci chen on 10/15/25.
//

// test_order_book.cpp
#include "order_book.h"   // your header
#include "order.h"        // for lob::Order and typedefs
#include <cassert>
#include <iostream>

int main() {
    lob::OrderBook ob;

    for (int i = 0; i < 1000; ++i) {
        lob::Order o{
                static_cast<lob::id_t>(i),
                static_cast<lob::ticks_t>(10000 + (i % 10)),
                static_cast<lob::qty_t>(5),
                (i % 2) ? lob::Side::Buy : lob::Side::Sell
        };
        ob.newOrder(o);
    }

    assert(ob.orderCount(static_cast<lob::ticks_t>(10000)) > 0);

    lob::PriceLevel bestB = ob.topOfBook(lob::Side::Buy);
    lob::PriceLevel bestA = ob.topOfBook(lob::Side::Sell);

    std::cout << "Tests passed. BestBid=" << bestB.price
              << " BestAsk=" << bestA.price << "\n";
    return 0;
}
