//
// Created by tianci chen on 10/1/25.
//
#include<map>
#include<memory>

#ifndef BUILD_LOCAL_ORDER_BOOK_MARKET_SNAPSHOT_H
#define BUILD_LOCAL_ORDER_BOOK_MARKET_SNAPSHOT_H
struct PriceLevel {
    double price;
    int quantity;

    PriceLevel(double p, int q) : price(p), quantity(q) {}
};

class MarketSnapshot {
public:
    void update_bid(double price, int qty);
    void update_ask(double price, int qty);
    const PriceLevel* get_best_bid() const;
    const PriceLevel* get_best_ask() const;
private:
    std::map<double, std::unique_ptr<PriceLevel>> bids; // sorted descending
    std::map<double, std::unique_ptr<PriceLevel>> asks; // sorted ascending
};


#endif //BUILD_LOCAL_ORDER_BOOK_MARKET_SNAPSHOT_H
