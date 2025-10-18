#include <vector>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "market_data.hpp"
#include "utils.hpp"
#include "strategy_virtual.hpp"
#include "strategy_crtp.hpp"

//=============================
// Free function baseline (control)
//=============================
inline double signal_free(const Quote& q, double a1, double a2) {
    const double mp  = microprice(q);
    const double m   = mid(q);
    const double imb = imbalance(q);
    return a1 * (mp - m) + a2 * imb;
}

//=============================
// Tick generators
//=============================
static void generate_ticks(std::vector<Quote>& out, uint32_t n, uint32_t seed) {
    out.resize(n);
    XorShift32 rng(seed);
    for (uint32_t i = 0; i < n; ++i) {
        double midp = rng.uniform(99.5, 100.5);     // around $100
        double sprd = rng.uniform(0.0005, 0.02);    // 0.05–2 cents
        double bq   = rng.uniform(100.0, 5000.0);   // sizes
        double aq   = rng.uniform(100.0, 5000.0);

        out[i] = Quote{
            midp - 0.5 * sprd,
            midp + 0.5 * sprd,
            bq,
            aq
        };
    }
}

// SoA layout for extension
struct QuotesSoA {
    std::vector<double> bid, ask, bid_qty, ask_qty;
};

static void generate_ticks_soa(QuotesSoA& q, uint32_t n, uint32_t seed) {
    q.bid.resize(n); q.ask.resize(n);
    q.bid_qty.resize(n); q.ask_qty.resize(n);
    XorShift32 rng(seed);
    for (uint32_t i = 0; i < n; ++i) {
        double midp = rng.uniform(99.5, 100.5);
        double sprd = rng.uniform(0.0005, 0.02);
        double bq   = rng.uniform(100.0, 5000.0);
        double aq   = rng.uniform(100.0, 5000.0);
        q.bid[i]     = midp - 0.5 * sprd;
        q.ask[i]     = midp + 0.5 * sprd;
        q.bid_qty[i] = bq;
        q.ask_qty[i] = aq;
    }
}

//=============================
// Bench harness (AoS)
//=============================
template <typename F>
static double run_bench(const char* name,
                        const std::vector<Quote>& ticks,
                        F&& func,
                        int iters)
{
    Timer t;
    t.start();
    volatile double sink = 0.0;
    for (int r = 0; r < iters; ++r) {
        for (const auto& q : ticks) {
            double s = func(q);
            do_not_optimize_away(s);
            sink += s * 1e-9; // keep it small
        }
    }
    double ns = t.stop_ns();
    std::printf("%-18s time: %.3f ms sink=%.6f\n", name, ns / 1e6, sink);
    return ns;
}

//=============================
// Bench harness (SoA extension)
//=============================
static double run_bench_soa(const QuotesSoA& q,
                            double a1, double a2,
                            int iters,
                            const char* name = "soa_baseline")
{
    Timer t;
    t.start();
    volatile double sink = 0.0;
    size_t n = q.bid.size();
    for (int r = 0; r < iters; ++r) {
        for (size_t i = 0; i < n; ++i) {
            const double denom = q.bid_qty[i] + q.ask_qty[i];
            const double inv   = 1.0 / denom;
            const double mp    = (q.bid[i] * q.ask_qty[i] + q.ask[i] * q.bid_qty[i]) * inv;
            const double m     = (q.bid[i] + q.ask[i]) * 0.5;
            const double imb   = (q.bid_qty[i] - q.ask_qty[i]) * inv;
            const double s     = a1 * (mp - m) + a2 * imb;
            do_not_optimize_away(s);
            sink += s * 1e-9;
        }
    }
    double ns = t.stop_ns();
    std::printf("%-18s time: %.3f ms sink=%.6f\n", name, ns / 1e6, sink);
    return ns;
}

//=============================
// Extension: Heterogeneous composition (Virtual multi)
//=============================
static double run_virtual_multi(const std::vector<Quote>& ticks, int iters) {
    SignalStrategyVirtual s1(0.70, 0.30);
    SignalStrategyVirtual s2(0.60, 0.40);
    SignalStrategyVirtual s3(0.80, 0.20);
    std::vector<IStrategy*> strategies = { &s1, &s2, &s3 };

    Timer t; t.start();
    volatile double sink = 0.0;
    for (int r = 0; r < iters; ++r) {
        for (const auto& q : ticks) {
            // 3 virtual calls per tick
            for (auto* s : strategies) {
                const double sig = s->on_tick(q);
                do_not_optimize_away(sig);
                sink += sig * 1e-9;
            }
        }
    }
    double ns = t.stop_ns();
    std::printf("%-18s time: %.3f ms sink=%.6f\n", "virtual_multi", ns/1e6, sink);
    return ns;
}

//=============================
// Extension: Heterogeneous composition (CRTP multi)
//=============================
template <typename... Strategies>
static double run_static_multi(const std::vector<Quote>& ticks, int iters, Strategies&&... s) {
    Timer t; t.start();
    volatile double sink = 0.0;
    for (int r = 0; r < iters; ++r) {
        for (const auto& q : ticks) {
            // 通过折叠表达式展开调用（可内联）
            ( (sink += s.on_tick(q) * 1e-9), ... );
        }
    }
    double ns = t.stop_ns();
    std::printf("%-18s time: %.3f ms sink=%.6f\n", "crtp_multi", ns/1e6, sink);
    return ns;
}

//=============================
// Reporting helpers
//=============================
static void report_one(const char* name, double ns, double ops_per_tick, double n_ticks_total) {
    const double total_ops  = n_ticks_total * ops_per_tick;
    const double ns_per_op  = ns / total_ops;
    const double tps        = 1e9 / ns_per_op;
    std::printf("%-18s ns/op:   %.3f  ops/sec: %.2f M  (%.1f ops/tick)\n",
                name, ns_per_op, tps / 1e6, ops_per_tick);
}

int main(int argc, char** argv) {
    // Parameters (can be overridden from CLI)
    uint32_t n_ticks = 10'000'000; // 10M
    int      iters   = 1;
    double   a1      = 0.75;
    double   a2      = 0.25;
    uint32_t seed    = 0xC001D00D;

    if (argc > 1) n_ticks = static_cast<uint32_t>(std::strtoul(argv[1], nullptr, 10));
    if (argc > 2) iters   = std::atoi(argv[2]);

    //========================
    // AoS benchmarks (main assignment)
    //========================
    std::cout << "Generating " << n_ticks << " ticks (AoS), iters=" << iters << "...\n";
    std::vector<Quote> ticks;
    generate_ticks(ticks, n_ticks, seed);

    auto ns_free = run_bench("free_function", ticks,
        [&](const Quote& q) { return signal_free(q, a1, a2); }, iters);

    SignalStrategyVirtual virt(a1, a2);
    IStrategy* s = &virt;
    auto ns_virtual = run_bench("virtual_call", ticks,
        [&](const Quote& q) { return s->on_tick(q); }, iters);

    SignalStrategyCRTP crtp(a1, a2);
    auto ns_crtp = run_bench("crtp_call", ticks,
        [&](const Quote& q) { return crtp.on_tick(q); }, iters);

    //========================
    // Extensions: Heterogeneous composition (3 strategies)
    //========================
    auto ns_vmulti = run_virtual_multi(ticks, iters);

    SignalStrategyCRTP c1(0.70, 0.30), c2(0.60, 0.40), c3(0.80, 0.20);
    auto ns_smulti = run_static_multi(ticks, iters, c1, c2, c3);

    //========================
    // Extensions: SoA layout
    //========================
    std::cout << "Generating " << n_ticks << " ticks (SoA), iters=" << iters << "...\n";
    QuotesSoA qsoa;
    generate_ticks_soa(qsoa, n_ticks, seed ^ 0x9E3779B9u); // 不同seed，避免cache复用侥幸

    auto ns_soa = run_bench_soa(qsoa, a1, a2, iters, "soa_baseline");

    //========================
    // Summary
    //========================
    std::puts("\n=== Summary ===");
    // 单策略：每 tick 1 次信号计算 → ops_per_tick = 1
    report_one("free_function", ns_free,   /*ops/tick*/ 1.0, static_cast<double>(n_ticks) * iters);
    report_one("virtual_call",  ns_virtual,/*ops/tick*/ 1.0, static_cast<double>(n_ticks) * iters);
    report_one("crtp_call",     ns_crtp,   /*ops/tick*/ 1.0, static_cast<double>(n_ticks) * iters);

    // 多策略：每 tick 3 次信号计算 → ops_per_tick = 3
    report_one("virtual_multi", ns_vmulti, /*ops/tick*/ 3.0, static_cast<double>(n_ticks) * iters);
    report_one("crtp_multi",    ns_smulti, /*ops/tick*/ 3.0, static_cast<double>(n_ticks) * iters);

    // SoA：每 tick 1 次信号计算 → ops_per_tick = 1
    report_one("soa_baseline",  ns_soa,    /*ops/tick*/ 1.0, static_cast<double>(n_ticks) * iters);

    std::puts("\nTip (Linux): taskset -c 0 ./hft 20000000 1");
    std::puts("perf stat -e cycles,instructions,branches,branch-misses ./hft 20000000 1");
    return 0;
}
