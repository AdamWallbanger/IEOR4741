//
// Created by tianci chen on 10/1/25.
//
#pragma once
#include <map>
#include <memory>
#include <iostream>
#include "market_snapshot.h"

#ifndef BUILD_LOCAL_ORDER_BOOK_ORDER_MANAGER_H
#define BUILD_LOCAL_ORDER_BOOK_ORDER_MANAGER_H

enum class OrderStatus { New, Filled, PartiallyFilled, Cancelled };
enum class Side { Buy, Sell };

struct MyOrder {
    int id;
    Side side;
    double price;
    int quantity;
    int filled = 0;
    OrderStatus status = OrderStatus::New;
};

class OrderManager{
public:
    int place_order(Side side, double price, int qty);
    void cancel(int id);
    void handle_fill(int id, int filled_qty);
    void print_active_orders() const;
    const MyOrder* get(int id) const;
private:
    std::map<int, std::unique_ptr<MyOrder>> orders;
};



#endif //BUILD_LOCAL_ORDER_BOOK_ORDER_MANAGER_H
