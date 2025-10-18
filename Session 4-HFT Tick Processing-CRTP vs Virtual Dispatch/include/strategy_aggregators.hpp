#pragma once
#include <vector>
#include <tuple>
#include "strategy_virtual.hpp"
#include "strategy_crtp.hpp"
#include "market_data.hpp"

// 虚函数路径：一次 tick 调用一组 IStrategy*（模拟插件式多策略组合）
inline double run_virtual_bundle(const Quote& q, const std::vector<IStrategy*>& strategies) {
    double acc = 0.0;
    for (auto* s : strategies) acc += s->on_tick(q);
    return acc;
}

// 静态路径（CRTP）：编译期组合多个策略，零开销展开
template <class... S>
struct StaticBundle {
    std::tuple<S...> ss;
    explicit StaticBundle(S... s) : ss(std::move(s)...) {}

    inline double on_tick(const Quote& q) {
        return std::apply([&](auto&... s){ return (s.on_tick(q) + ...); }, ss);
    }
};
