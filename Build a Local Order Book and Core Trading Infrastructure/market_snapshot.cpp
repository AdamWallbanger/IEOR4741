//
// Created by tianci chen on 10/1/25.
//
#include"market_snapshot.h"
void MarketSnapshot :: update_bid(double price, int qty){
    if(qty <= 0) {
        bids.erase(price);
        return;
    }
    auto it = bids.find(price);
    if (it == bids.end()){
        bids.emplace(price, std :: make_unique<PriceLevel>(price, qty));
    }
    else{
        it->second->price = price;
        it ->second->quantity = qty;
    }
}
void MarketSnapshot :: update_ask(double price, int qty){
    if(qty <= 0) {
        asks.erase(price);
        return;
    }
    auto it=asks.find(price);
    if(it==asks.end()){
        asks.emplace(price, std :: make_unique<PriceLevel>(price,qty));
    }
    else{
        it->second->price = price;
        it->second->quantity = qty;
    }

}
const PriceLevel* MarketSnapshot ::  get_best_bid() const{
    if (bids.empty()){
        return nullptr;
    }
    return bids.begin() -> second.get();
}
const PriceLevel* MarketSnapshot ::  get_best_ask() const{
    if (asks.empty()){
        return nullptr;
    }
    return asks.begin() -> second.get();
}