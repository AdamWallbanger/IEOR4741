//
// Created by tianci chen on 10/15/25.
//
// benchmark.cpp

// benchmark.cpp

// ---------- Select implementation ----------
#if defined(USE_VECTOR_BOOK)
#include "order_book_vector.h"
  using Book = lob::OrderBookVector;
#elif defined(USE_MAP_BOOK)
#include "order_book_map.h"
  using Book = lob::OrderBookMap;
#else
#include "order_book.h"
using Book = lob::OrderBook;
#endif


#include <chrono>
#include <iostream>
#include <random>
#include <vector>

int main() {
    Book ob(10'000'000);

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> price(10000, 10050);

    const std::size_t N = 10'000'000;
    std::vector<lob::Order> orders;
    orders.reserve(N);

    for (std::size_t i = 0; i < N; ++i) {
        orders.push_back({static_cast<lob::id_t>(i),
                          static_cast<lob::ticks_t>(price(rng)),
                          static_cast<lob::qty_t>(10),
                          (i % 2) ? lob::Side::Buy : lob::Side::Sell});
    }

    using clock = std::chrono::high_resolution_clock;

    // Insert
    auto t0 = clock::now();
    for (auto& o : orders) ob.newOrder(o);
    auto t1 = clock::now();

    // Amend (10%)
    for (std::size_t i = 0; i < N; i += 10) ob.amendOrder(orders[i].id, orders[i].qty + 5);
    auto t2 = clock::now();

    // Delete (same 10%)
    for (std::size_t i = 0; i < N; i += 10) ob.deleteOrder(orders[i].id);
    auto t3 = clock::now();

    // Top-of-book queries
    for (std::size_t i = 0; i < 1'000'000; ++i) {
        if (i & 1) (void)ob.topOfBook(lob::Side::Buy);
        else       (void)ob.topOfBook(lob::Side::Sell);
    }
    auto t4 = clock::now();

    const double insert_s = std::chrono::duration<double>(t1 - t0).count();
    const double amend_s  = std::chrono::duration<double>(t2 - t1).count();
    const double delete_s = std::chrono::duration<double>(t3 - t2).count();
    const double top_s    = std::chrono::duration<double>(t4 - t3).count();

    std::cout.setf(std::ios::fixed);
    std::cout.precision(6);
    std::cout << "Insert: " << (N / insert_s) / 1e6 << " Mops/s\n";
    std::cout << "Amend:  " << ((N / 10) / amend_s) / 1e6 << " Mops/s\n";
    std::cout << "Delete: " << ((N / 10) / delete_s) / 1e6 << " Mops/s\n";
    std::cout << "Top-of-book latency: " << (top_s / 1'000'000) * 1e9 << " ns/query\n";

    return 0;
}
