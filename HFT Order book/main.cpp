#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <chrono>
#include <atomic>
#include <cassert>
#include <random>
using namespace std;
std::atomic<int> orderCount(0);

void addAtomicOrder() {
    orderCount.fetch_add(1, std::memory_order_relaxed);
}
struct Order {
    std::string id;  // String-based order ID
    double price;
    int quantity;
    bool isBuy;
};

class OrderBook {
private:
    std::map<double, std::unordered_map<std::string, Order>> orderLevels;

public:
    std::unordered_map<std::string, Order> orderLookup;
    void addOrder(const std::string& id, double price, int quantity, bool isBuy) {
        Order order = {id, price, quantity, isBuy};
        orderLevels[price][id] = order;
        orderLookup[id] = order;
    }
    void modifyOrder(const std::string& id, double newPrice, int newQuantity) {
        if (orderLookup.find(id) != orderLookup.end()) {
            Order oldOrder = orderLookup[id];
            orderLevels[oldOrder.price].erase(id);
            addOrder(id, newPrice, newQuantity, oldOrder.isBuy);
        }
    }

    void deleteOrder(const std::string& id) {
        if (orderLookup.find(id) != orderLookup.end()) {
            Order order = orderLookup[id];
            orderLevels[order.price].erase(id);
            orderLookup.erase(id);
        }
    }
};

void testAddOrder() {
    OrderBook book;
    book.addOrder("ORD001", 50.10, 100, true);

    assert(book.orderLookup.count("ORD001") == 1);  // Order should exist
}

void stressTest(OrderBook& book, int numOrders) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> priceDist(50.0, 100.0);
    std::uniform_int_distribution<int> quantityDist(1, 500);

    for (int i = 0; i < numOrders; ++i) {
        std::string id = "ORD" + std::to_string(i);
        double price = priceDist(rng);
        int quantity = quantityDist(rng);

        book.addOrder(id, price, quantity, true);
        price = priceDist(rng);
        quantity = quantityDist(rng);
        book.modifyOrder(id, price, quantity);
        book.deleteOrder(id);
    }
}

void benchmark(int numOrders)
{
    OrderBook book;
    auto start = std::chrono::high_resolution_clock::now();
    stressTest(book, numOrders);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Execution time: " << elapsed.count() << " seconds" << std::endl;
}

int main()
{
    int test_num[6] = {1000, 5000, 10000, 50000, 100000,1000000};
    for (int i=0;i<6;i++)
    {
        benchmark(test_num[i]);
    }
    return 0;
}