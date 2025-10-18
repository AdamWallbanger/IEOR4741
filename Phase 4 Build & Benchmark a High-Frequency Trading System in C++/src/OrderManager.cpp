#include "../include/OrderBook.hpp"
#include "../include/OrderManager.hpp"
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

    Ord* add(OrderIdType id, PriceType px, int qty, bool is_buy) {
        auto up = std::make_unique<Ord>(Ord{id, px, qty, is_buy});
        Ord* p = up.get();
        if (is_buy) store_.buys.emplace(px, p); else store_.sells.emplace(px, p);
        store_.owned.push_back(std::move(up));
        by_id_[id] = p;
        st_[id] = Status{};
        return p;
    }

    bool cancel(OrderIdType id) {
        auto it = st_.find(id);
        if (it == st_.end() || it->second.state == State::Filled) return false;
        it->second.state = State::Canceled;
        return true;
    }

    int fill(OrderIdType id, int qty) {
        auto si = st_.find(id); if (si == st_.end()) return 0;
        auto oi = by_id_.find(id); if (oi == by_id_.end()) return 0;
        if (si->second.state == State::Canceled || si->second.state == State::Filled) return 0;
        int rem = oi->second->quantity - si->second.filled;
        int take = qty < rem ? qty : rem;
        si->second.filled += take;
        si->second.state = (si->second.filled == oi->second->quantity) ? State::Filled
                                                                       : State::PartiallyFilled;
        return take;
    }

    Status status(OrderIdType id) const {
        auto it = st_.find(id);
        return it == st_.end() ? Status{} : it->second;
    }
};