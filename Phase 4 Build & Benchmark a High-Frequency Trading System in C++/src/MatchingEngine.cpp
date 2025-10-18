#include <vector>
#include <chrono>
#include "../include/MatchingEngine.hpp"
using namespace std;

template <typename PriceType, typename OrderIdType>
vector<Trade<PriceType, OrderIdType>>
match_after_add(
    OrderStore<PriceType, OrderIdType>& book,
    OrderManagement<PriceType, OrderIdType>& oms,
    OrderIdType new_order_id,
    chrono::high_resolution_clock::time_point arrival_ts // when the new order arrived
) {
    using Ord   = Order<PriceType, OrderIdType>;
    using TRec  = Trade<PriceType, OrderIdType>;
    using Clock = chrono::high_resolution_clock;

    vector<TRec> out;

    auto remaining = [&](const Ord* o) {
        auto s = oms.status(o->id);
        return o->quantity - s.filled;
    };
    auto inactive = [&](const Ord* o) {
        auto s = oms.status(o->id);
        return s.state == decltype(s.state)::Filled || s.state == decltype(s.state)::Canceled;
    };

    auto best_bid_it = [&]() {
        while (!book.buys.empty()) {
            auto it = std::prev(book.buys.end());
            if (inactive(it->second) || remaining(it->second) <= 0) { book.buys.erase(it); continue; }
            return it;
        }
        return book.buys.end();
    };
    auto best_ask_it = [&]() {
        while (!book.sells.empty()) {
            auto it = book.sells.begin();
            if (inactive(it->second) || remaining(it->second) <= 0) { book.sells.erase(it); continue; }
            return it;
        }
        return book.sells.end();
    };

    for (;;) {
        auto bit = best_bid_it();
        auto ait = best_ask_it();
        if (bit == book.buys.end() || ait == book.sells.end()) break;

        Ord* b = bit->second;
        Ord* s = ait->second;
        if (b->price < s->price) break; // no cross

        int qty = min(remaining(b), remaining(s));
        PriceType px = s->price;        // simple: trade at best ask

        oms.fill(b->id, qty);
        oms.fill(s->id, qty);

        long long lat = -1;
        if (b->id == new_order_id || s->id == new_order_id) {
            lat = chrono::duration_cast<chrono::nanoseconds>(Clock::now() - arrival_ts).count();
        }

        out.push_back(TRec{b->id, s->id, px, qty, lat});

        if (remaining(b) == 0) book.buys.erase(bit);
        if (remaining(s) == 0) book.sells.erase(ait);
    }

    return out;
}