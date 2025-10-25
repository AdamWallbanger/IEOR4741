#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <fstream>
#include <atomic>
#include <vector>
#include <algorithm>
#include <cstdlib>

using namespace std;
using namespace std::chrono;

// ---------- Data Structures ----------
struct PriceUpdate {
    double price;
    steady_clock::time_point timestamp; // feed timestamp
};

// NOTE: Added src_ts so we can measure true end-to-end latency (feed -> router).
struct Order {
    string side; // "BUY" or "SELL"
    double price;
    steady_clock::time_point timestamp; // created at strategy
    steady_clock::time_point src_ts;    // original feed timestamp
};

// ---------- Shared Queues ----------
queue<PriceUpdate> priceQueue;
queue<Order>       orderQueue;

mutex priceMutex, orderMutex;
condition_variable priceCV, orderCV;

atomic<bool> running{true};

// ---------- Market Data Feed ----------
void marketDataFeed(int rate_per_sec) {
    // Sleep interval per tick
    auto sleep_ns = (rate_per_sec > 0) ? nanoseconds(1'000'000'000LL / rate_per_sec)
                                       : nanoseconds(0);

    while (running) {
        if (sleep_ns.count() > 0) {
            this_thread::sleep_for(sleep_ns);
        } // else "as fast as possible" (not used in your benchmarks)

        PriceUpdate update{100.0 + double(rand() % 10), steady_clock::now()};
        {
            lock_guard<mutex> lock(priceMutex);
            priceQueue.push(update);
        }
        priceCV.notify_one();
    }
}

// ---------- Strategy Engine ----------
void strategyEngine(double threshold = 2.0) {
    // We track lastPrice across updates
    bool has_last = false;
    double lastPrice = 0.0;

    while (running) {
        unique_lock<mutex> lock(priceMutex);
        priceCV.wait(lock, [] { return !priceQueue.empty() || !running; });
        if (!running) break;

        PriceUpdate update = priceQueue.front();
        priceQueue.pop();
        lock.unlock();

        if (!has_last) {
            lastPrice = update.price;
            has_last = true;
            continue;
        }

        double delta = update.price - lastPrice;

        if (std::abs(delta) > threshold) {
            Order order{
                    (delta < 0 ? "BUY" : "SELL"),
                    update.price,
                    steady_clock::now(), // strategy timestamp
                    update.timestamp     // original feed timestamp
            };
            {
                lock_guard<mutex> olock(orderMutex);
                orderQueue.push(order);
            }
            orderCV.notify_one();
        }

        lastPrice = update.price;
    }
}

// ---------- Order Router (metrics + logging) ----------
void orderRouter(ofstream& logFile,
                 vector<long long>& e2e_lat_us,
                 vector<long long>& s2r_lat_us,
                 atomic<size_t>& total_orders)
{
    // CSV header
    logFile << "Side,Price,EndToEnd_us,StrategyToRouter_us,RouterTs_us\n";

    while (running) {
        unique_lock<mutex> lock(orderMutex);
        orderCV.wait(lock, [] { return !orderQueue.empty() || !running; });
        if (!running) break;

        Order order = orderQueue.front();
        orderQueue.pop();
        lock.unlock();

        auto now = steady_clock::now();

        // End-to-end latency: feed -> router
        auto e2e = duration_cast<microseconds>(now - order.src_ts).count();
        // Strategy-to-router latency: strategy -> router
        auto s2r = duration_cast<microseconds>(now - order.timestamp).count();
        auto router_ts = duration_cast<microseconds>(now.time_since_epoch()).count();

        // Append to vectors for stats (non-thread-safe; only router thread writes)
        e2e_lat_us.push_back(e2e);
        s2r_lat_us.push_back(s2r);
        total_orders.fetch_add(1, memory_order_relaxed);

        // Logging (dominant cost at higher rates)
        logFile << order.side << "," << order.price << ","
                << e2e << "," << s2r << "," << router_ts << "\n";
    }
}

// ---------- Helper: print summary stats ----------
static void print_stats(const char* name, const vector<long long>& v) {
    if (v.empty()) {
        cout << name << ": no samples\n";
        return;
    }
    vector<long long> a = v;
    sort(a.begin(), a.end());
    auto pct = [&](double p)->long long {
        size_t idx = (size_t)((p/100.0) * (a.size()-1));
        return a[idx];
    };
    long double sum = 0;
    for (auto x : a) sum += x;
    long double avg = sum / a.size();

    cout << name << " (microseconds)\n";
    cout << "  count=" << a.size()
         << " avg=" << (double)avg
         << " p50=" << pct(50) << " p95=" << pct(95)
         << " p99=" << pct(99) << " max=" << a.back() << "\n";
}

// ---------- Main ----------
int main(int argc, char** argv) {
    // Params: rate_per_sec, threshold, run_seconds
    int    rate_per_sec = 1000;
    double threshold    = 2.0;
    int    run_seconds  = 5;

    if (argc > 1) rate_per_sec = std::max(0, atoi(argv[1]));
    if (argc > 2) threshold    = atof(argv[2]);
    if (argc > 3) run_seconds  = std::max(1, atoi(argv[3]));

    // Seed RNG for repeatability (optional)
    srand(42);

    ofstream logFile("orders.csv");

    vector<long long> e2e_lat_us;
    vector<long long> s2r_lat_us;
    e2e_lat_us.reserve(1 << 20); // reserve some space to reduce reallocs
    s2r_lat_us.reserve(1 << 20);

    atomic<size_t> total_orders{0};

    auto t0 = steady_clock::now();

    thread feedThread(marketDataFeed, rate_per_sec);
    thread strategyThread(strategyEngine, threshold);
    thread routerThread(orderRouter, ref(logFile),
                        ref(e2e_lat_us), ref(s2r_lat_us),
                        ref(total_orders));

    // Fixed-duration run
    this_thread::sleep_for(seconds(run_seconds));
    running = false;

    // Unblock waiters
    priceCV.notify_all();
    orderCV.notify_all();

    // Join
    feedThread.join();
    strategyThread.join();
    routerThread.join();

    auto elapsed_s = duration_cast<seconds>(steady_clock::now() - t0).count();
    if (elapsed_s == 0) elapsed_s = 1; // safety

    logFile.close();

    // Throughput
    double orders_per_sec = double(total_orders.load(memory_order_relaxed)) / double(elapsed_s);

    cout << "==================== Benchmark Summary ====================\n";
    cout << "Market Rate     : " << rate_per_sec << " updates/sec\n";
    cout << "Threshold       : " << threshold << "\n";
    cout << "Run Duration    : " << elapsed_s << " s\n";
    cout << "Threads         : 3 (feed, strategy, router)\n";
    cout << "Orders processed: " << total_orders.load() << "\n";
    cout << "Orders/sec      : " << orders_per_sec << "\n\n";

    print_stats("End-to-End latency (feed->router)", e2e_lat_us);
    print_stats("Strategy->Router latency",          s2r_lat_us);

    cout << "\nCSV written to orders.csv with per-order latencies.\n";
    cout << "===========================================================\n";

    return 0;
}
