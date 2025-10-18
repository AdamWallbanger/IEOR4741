#include "../include/OrderBook.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
using namespace std;

template <typename PriceType, typename OrderIdType>
class OrderManagement {
public:
    using Ord = Order<PriceType, OrderIdType>;
    enum class State { New, PartiallyFilled, Filled, Canceled };
    struct Status { int filled = 0; State state = State::New; };

private:
    OrderStore<PriceType, OrderIdType>& store_;
    unordered_map<OrderIdType, Ord*> by_id_;
    unordered_map<OrderIdType, Status> st_;

public:
    explicit OrderManagement(OrderStore<PriceType, OrderIdType>& s) : store_(s) {}
    Ord* add(OrderIdType id, PriceType px, int qty, bool is_buy);
    bool cancel(OrderIdType id);
    int fill(OrderIdType id, int qty);
    Status status(OrderIdType id);
};