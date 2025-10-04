#include <iostream>
#include <fstream>
#include <unordered_map>
#include "market_snapshot.h"
#include "order_manager.h"
#include "feed_parser.h"

static void log_best_changes(std::ofstream& log,
                             const PriceLevel* prevBid, const PriceLevel* prevAsk,
                             const PriceLevel* curBid,  const PriceLevel* curAsk)
{
    if (prevBid != curBid) {
        if (curBid) {
            log << "[Market] Best Bid: " << curBid->price << " x " << curBid->quantity << "\n";
        } else if (prevBid) {
            log << "[Market] Best Bid: " << prevBid->price << " removed\n";
        }
    }
    if (prevAsk != curAsk) {
        if (curAsk) {
            if (!prevAsk)
                log << "[Market] Best Ask: " << curAsk->price << " x " << curAsk->quantity << "\n";
            else
                log << "[Market] New Ask: " << curAsk->price << " x " << curAsk->quantity << "\n";
        } else if (prevAsk) {
            log << "[Market] Best Ask: " << prevAsk->price << " removed\n";
        }
    }
}

int main(){
    std::ofstream log("output.log");
    MarketSnapshot snapshot;
    OrderManager om;

    const double sell_threshold = 100.00;
    const int    order_qty      = 50;

    std::unordered_map<int,int> initial_qty;

    const PriceLevel* lastBid = nullptr;
    const PriceLevel* lastAsk = nullptr;

    auto feed = load_feed("sample_feed.txt");
    for (const auto& ev : feed) {
        switch (ev.type) {
            case FeedType::BID:
                snapshot.update_bid(ev.price, ev.quantity);
                break;
            case FeedType::ASK:
                snapshot.update_ask(ev.price, ev.quantity);
                break;
            case FeedType::EXECUTION: {
                log << "[Execution] Order " << ev.order_id << " filled: " << ev.quantity << "\n";
                om.handle_fill(ev.order_id, ev.quantity);

                // If still active → partial; if removed → completed
                if (const MyOrder* o = om.get(ev.order_id)) {
                    log << "[Order] Order " << o->id << " partially filled: "
                        << o->filled << " / " << o->quantity << "\n";
                } else {
                    int Q = initial_qty[ev.order_id];
                    log << "[Order] Order " << ev.order_id << " completed ("
                        << Q << " / " << Q << ") and removed\n";
                }
                break;
            }
            default: break;
        }

        const PriceLevel* curBid = snapshot.get_best_bid();
        const PriceLevel* curAsk = snapshot.get_best_ask();
        log_best_changes(log, lastBid, lastAsk, curBid, curAsk);
        lastBid = curBid;
        lastAsk = curAsk;

        if (curBid && curBid->price > sell_threshold) {
            int id = om.place_order(Side::Sell, curBid->price, order_qty);
            initial_qty[id] = order_qty;
            log << "[Strategy] Placing SELL order at " << curBid->price
                << " x " << order_qty << " (ID = " << id << ")\n";
        }
    }

    std::cout << "Active orders:\n";
    om.print_active_orders();
    std::cout << "Wrote log to output.log\n";
    return 0;
}
