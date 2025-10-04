//
// Created by tianci chen on 10/1/25.
//
#include"order_manager.h"

static const char* to_string(Side s) {
    return s == Side::Buy ? "BUY" : "SELL";
}
static const char* to_string(OrderStatus st) {
    switch (st) {
        case OrderStatus::New:             return "New";
        case OrderStatus::Filled:          return "Filled";
        case OrderStatus::PartiallyFilled: return "PartiallyFilled";
        case OrderStatus::Cancelled:       return "Cancelled";
    }
    return "Unknown";
}

static int next_order_id() {
    static int id = 1;
    return id++;
}

int OrderManager::place_order(Side side, double price, int qty) {
    const int id = next_order_id();

    auto ord = std::make_unique<MyOrder>();
    ord->id       = id;
    ord->side     = side;
    ord->price    = price;
    ord->quantity = qty;
    ord->filled   = 0;
    ord->status   = OrderStatus::New;

    orders.emplace(id, std::move(ord));
    return id;
}

void OrderManager::cancel(int id) {
    auto it = orders.find(id);
    if (it == orders.end()) return;
    it->second->status = OrderStatus::Cancelled;
    orders.erase(it);
}

void OrderManager::handle_fill(int id, int filled_qty) {
    auto it = orders.find(id);
    if (it == orders.end() || filled_qty <= 0) return;

    MyOrder* o = it->second.get();
    o->filled += filled_qty;
    if (o->filled >= o->quantity) {
        o->filled = o->quantity;
        o->status = OrderStatus::Filled;
        orders.erase(it);
    } else {
        o->status = OrderStatus::PartiallyFilled;
    }
}

void OrderManager::print_active_orders() const {
    for (const auto& kv : orders) {
        const MyOrder& o = *kv.second;
        std::cout << "Order " << o.id << " "
                  << to_string(o.side)
                  << " @" << o.price
                  << " qty=" << o.quantity
                  << " filled=" << o.filled
                  << " status=" << to_string(o.status)
                  << "\n";
    }
}

const MyOrder* OrderManager::get(int id) const{
    auto it = orders.find(id);
    return it==orders.end()? nullptr : it->second.get();
}