// hft_assignment.cpp (atomic-sink version; C++20)
// Build (macOS): clang++ -std=c++20 -O3 -march=native -flto -DNDEBUG hft_assignment.cpp -o hft_bench
// Build (Windows): cl /O2 /std:c++20 /DNDEBUG hft_assignment.cpp

#include <algorithm>
#include <array>
#include <atomic>   // <— added
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <vector>

using Clock = std::chrono::high_resolution_clock;

struct Order {
    uint64_t id;
    int      side;     // 0 or 1
    int      qty;
    int      price;
    int      payload[2];
};

// simple mix to form checksum
static inline uint64_t mix64(uint64_t x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

struct WorkPolicy {
    static constexpr size_t BOOK_SZ = 64;
    static thread_local int s_book_qty[BOOK_SZ];
    static thread_local int s_book_px [BOOK_SZ];
    static thread_local uint64_t s_branch_counter;

    static inline uint64_t do_work(Order& o) {
        uint64_t h = o.id;
        h ^= static_cast<uint64_t>(o.side) * 0x9e3779b97f4a7c15ULL;
        h += static_cast<uint64_t>(o.qty) * 13u;
        h ^= static_cast<uint64_t>(o.price) * 101u;
        h ^= static_cast<uint64_t>(o.payload[0]) * 17u;
        h += static_cast<uint64_t>(o.payload[1]) * 23u;

        size_t idx = (static_cast<size_t>(o.id) + static_cast<size_t>(o.side) * 7) & (BOOK_SZ - 1);
        s_book_qty[idx] = o.qty;    // write 1
        s_book_px[idx]  = o.price;  // write 2

        if ((o.price & 1) != 0) s_branch_counter += 1;

        return mix64(h);
    }
};

thread_local int WorkPolicy::s_book_qty[WorkPolicy::BOOK_SZ] = {};
thread_local int WorkPolicy::s_book_px [WorkPolicy::BOOK_SZ] = {};
thread_local uint64_t WorkPolicy::s_branch_counter = 0;

// ---- Virtual impl ----
struct Processor {
    virtual ~Processor() = default;
    virtual uint64_t process(Order& o) = 0;
};
struct StrategyA_V : Processor {
    uint64_t process(Order& o) override {
        o.qty   = (o.qty   * 3 + 1) & 1023;
        o.price = (o.price * 5 + 7) & 65535;
        return WorkPolicy::do_work(o);
    }
};
struct StrategyB_V : Processor {
    uint64_t process(Order& o) override {
        o.qty   = (o.qty   * 7 + 2) & 2047;
        o.price = (o.price * 9 + 3) & 65535;
        return WorkPolicy::do_work(o);
    }
};

// ---- Non-virtual impl ----
struct StrategyA_NV {
    uint64_t run(Order& o) {
        o.qty   = (o.qty   * 3 + 1) & 1023;
        o.price = (o.price * 5 + 7) & 65535;
        return WorkPolicy::do_work(o);
    }
};
struct StrategyB_NV {
    uint64_t run(Order& o) {
        o.qty   = (o.qty   * 7 + 2) & 2047;
        o.price = (o.price * 9 + 3) & 65535;
        return WorkPolicy::do_work(o);
    }
};

enum class Pattern : int { HomogeneousA=0, MixedRandom=1, Bursty=2 };

static const char* pattern_name(Pattern p) {
    switch (p) {
        case Pattern::HomogeneousA: return "homogeneous";
        case Pattern::MixedRandom:  return "mixed";
        case Pattern::Bursty:       return "bursty";
    }
    return "unknown";
}
static const char* impl_name(bool virtual_impl) { return virtual_impl ? "virtual" : "nonvirtual"; }

static void make_orders(std::vector<Order>& orders, size_t N) {
    orders.resize(N);
    for (size_t i = 0; i < N; ++i) {
        orders[i].id      = static_cast<uint64_t>(i + 1);
        orders[i].side    = static_cast<int>(i & 1);
        orders[i].qty     = static_cast<int>(1 + (i % 97));
        orders[i].price   = static_cast<int>(10'000 + (i % 1000));
        orders[i].payload[0] = static_cast<int>((i * 31u) & 0xffff);
        orders[i].payload[1] = static_cast<int>((i * 17u) & 0xffff);
    }
}

static void make_assign(std::vector<uint8_t>& assign, Pattern p, size_t N, uint64_t seed=42) {
    assign.resize(N);
    if (p == Pattern::HomogeneousA) {
        std::fill(assign.begin(), assign.end(), 0u);
    } else if (p == Pattern::MixedRandom) {
        std::mt19937_64 rng(seed);
        std::uniform_int_distribution<int> bit(0,1);
        for (size_t i=0;i<N;++i) assign[i] = static_cast<uint8_t>(bit(rng));
    } else {
        const int A = 64, B = 16, P = A + B;
        for (size_t i=0;i<N;++i) {
            int t = static_cast<int>(i % P);
            assign[i] = static_cast<uint8_t>(t < A ? 0 : 1);
        }
    }
}

struct RunResult {
    uint64_t elapsed_ns;
    double   ops_per_sec;
    uint64_t checksum;
};

// global atomic sink to publish warmup work (prevents elision, no fences)
static std::atomic<uint64_t> g_warmup_sink{0};

template <typename F>
static void warmup(F&& f, size_t warmup_ops) {
    uint64_t local = 0;
    for (size_t i=0;i<warmup_ops;++i) local += f();
    g_warmup_sink.fetch_add(local, std::memory_order_relaxed);
}

static RunResult run_virtual(const std::vector<Order>& base,
                             const std::vector<uint8_t>& assign)
{
    std::vector<Order> orders = base;
    StrategyA_V a; StrategyB_V b;
    std::vector<Processor*> procs = { &a, &b };

    std::atomic<uint64_t> checksum{0};
    auto t0 = Clock::now();
    for (size_t i=0;i<orders.size();++i) {
        Processor* p = procs[assign[i]];
        checksum.fetch_add(p->process(orders[i]), std::memory_order_relaxed);
    }
    auto t1 = Clock::now();
    uint64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    double ops_s = (ns == 0) ? 0.0 : (1e9 * static_cast<double>(orders.size()) / static_cast<double>(ns));
    return { ns, ops_s, checksum.load(std::memory_order_relaxed) };
}

static RunResult run_nonvirtual(const std::vector<Order>& base,
                                const std::vector<uint8_t>& assign)
{
    std::vector<Order> orders = base;
    StrategyA_NV a; StrategyB_NV b;

    std::atomic<uint64_t> checksum{0};
    auto t0 = Clock::now();
    for (size_t i=0;i<orders.size();++i) {
        if (assign[i] == 0) checksum.fetch_add(a.run(orders[i]), std::memory_order_relaxed);
        else                checksum.fetch_add(b.run(orders[i]), std::memory_order_relaxed);
    }
    auto t1 = Clock::now();
    uint64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    double ops_s = (ns == 0) ? 0.0 : (1e9 * static_cast<double>(orders.size()) / static_cast<double>(ns));
    return { ns, ops_s, checksum.load(std::memory_order_relaxed) };
}

static double median_ns(std::vector<uint64_t> v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    size_t mid = v.size()/2;
    if (v.size() % 2 == 1) return static_cast<double>(v[mid]);
    return 0.5 * (static_cast<double>(v[mid-1]) + static_cast<double>(v[mid]));
}

int main(int argc, char** argv) {
    size_t N = (argc >= 2) ? static_cast<size_t>(std::stoull(argv[1])) : 500'000;
    int repeats = (argc >= 3) ? std::stoi(argv[2]) : 10;
    std::string out_csv = (argc >= 4) ? argv[3] : "";

    const size_t WARMUP_OPS = 1'000'000;

    std::vector<Order> base;
    make_orders(base, N);

    std::string header = "pattern,impl,repeat,orders,elapsed_ns,ops_per_sec,checksum\n";
    std::cout << header;

    struct Agg { std::vector<uint64_t> elapsed; std::vector<double> ops; };
    Agg agg[3][2];

    for (int p_i = 0; p_i < 3; ++p_i) {
        Pattern p = static_cast<Pattern>(p_i);
        std::vector<uint8_t> assign;
        make_assign(assign, p, N, /*seed=*/12345);

        // Warmup both impls
        {
            std::vector<Order> warm_base; make_orders(warm_base, 100'000);
            std::vector<uint8_t> warm_assign; make_assign(warm_assign, p, 100'000, 777);
            StrategyA_V av; StrategyB_V bv;
            StrategyA_NV an; StrategyB_NV bn;

            size_t wcnt = 0;
            warmup([&]{
                Order o = warm_base[wcnt % warm_base.size()];
                Processor* procs[2] = { &av, &bv };
                uint64_t r = procs[warm_assign[wcnt % warm_assign.size()]]->process(o);
                ++wcnt; return r;
            }, WARMUP_OPS/2);

            wcnt = 0;
            warmup([&]{
                Order o = warm_base[wcnt % warm_base.size()];
                uint64_t r = (warm_assign[wcnt % warm_assign.size()] == 0) ? an.run(o) : bn.run(o);
                ++wcnt; return r;
            }, WARMUP_OPS/2);
        }

        for (int r = 0; r < repeats; ++r) {
            auto rv = run_virtual(base, assign);
            agg[p_i][1].elapsed.push_back(rv.elapsed_ns);
            agg[p_i][1].ops.push_back(rv.ops_per_sec);
            std::cout << pattern_name(p) << ",virtual," << r << "," << N << ","
                      << rv.elapsed_ns << "," << rv.ops_per_sec << "," << rv.checksum << "\n";

            auto rn = run_nonvirtual(base, assign);
            agg[p_i][0].elapsed.push_back(rn.elapsed_ns);
            agg[p_i][0].ops.push_back(rn.ops_per_sec);
            std::cout << pattern_name(p) << ",nonvirtual," << r << "," << N << ","
                      << rn.elapsed_ns << "," << rn.ops_per_sec << "," << rn.checksum << "\n";

            std::cerr << "[checksum] pattern=" << pattern_name(p)
                      << " virtual=" << rv.checksum
                      << " nonvirtual=" << rn.checksum << "\n";
        }
    }

    if (!out_csv.empty()) {
        std::cerr << "Tip: redirect stdout to a file to save CSV, e.g., ./hft_bench > " << out_csv << "\n";
    }

    for (int p_i=0;p_i<3;++p_i) {
        for (int impl=0;impl<2;++impl) {
            auto v = agg[p_i][impl].elapsed;
            auto o = agg[p_i][impl].ops;
            if (v.empty()) continue;
            uint64_t best = *std::min_element(v.begin(), v.end());
            double med = median_ns(v);
            double best_ops = *std::max_element(o.begin(), o.end());
            double med_ops  = (static_cast<double>(base.size()) * 1e9) / med;
            std::cerr << "[summary] pattern=" << pattern_name(static_cast<Pattern>(p_i))
                      << " impl=" << (impl==1?"virtual":"nonvirtual")
                      << " best_ns=" << best << " med_ns=" << (uint64_t)med
                      << " best_ops_s=" << best_ops << " med_ops_s=" << med_ops << "\n";
        }
    }
    return 0;
}
