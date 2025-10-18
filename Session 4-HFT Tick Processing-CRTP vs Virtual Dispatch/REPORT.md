# HFT Tick Processing: CRTP vs Virtual Dispatch — Report

## Setup
- **Machine:** Windows laptop
- **CPU:** 12th Gen Intel(R) Core(TM) i9-12900H — 14 cores / 20 threads, MaxClock 2.5 GHz
- **Compiler:** g++ (MSYS2) 14.2.0
- **Flags:** `-O3 -DNDEBUG -march=native -fno-exceptions -fno-rtti`
- **Workload:** synthetic top-of-book quotes, N = 10,000,000 ticks, iters = 1
- **Notes:** Run on native Windows (no Linux `perf` counters in this run).

## Results — Base Variants
| Variant             | time (ms) | ns/tick | ticks/sec (M) |
|---------------------|----------:|--------:|--------------:|
| free_function (AoS) | 23.137    | 2.314   | 432.22        |
| virtual_call (AoS)  | 25.756    | 2.576   | 388.27        |
| crtp_call (AoS)     | 24.323    | 2.432   | 411.13        |

**Relative differences (vs free):**
- virtual: **+11.3%** (2.576 vs 2.314 ns/tick)
- CRTP: **+5.1%**  (2.432 vs 2.314 ns/tick)

## Results — Extensions (Observed, optional)
| Variant              | ns/tick | ticks/sec (M) |
|----------------------|--------:|--------------:|
| virtual_bundle(2)    | 3.719   | 268.88        |
| crtp_bundle(2)       | 2.653   | 376.98        |
| free_recip (AoS)     | 2.592   | 385.82        |
| free_function (SoA)  | 2.289   | 436.79        |

**Highlights:**
- `virtual_bundle(2)` vs `crtp_bundle(2)`: **+40.2%** slower (3.719 vs 2.653 ns/tick)
- SoA vs AoS (free): **~+1.1%** faster on this machine (2.289 vs 2.314 ns/tick)
- Reciprocal variant (reduce divisions) was **~+12.0%** slower here → “branchless/reciprocal” is workload/CPU dependent

## Interpretation
- **CRTP ≈ free** in tight loops: both allow full inlining and constant propagation; no vtable/indirect-call barriers.
- **Virtual dispatch** is measurably slower due to the *indirect call*, which:
  1) blocks inlining across the call site,
  2) introduces an indirect branch (harder to predict),
  3) can worsen code layout vs hot-path I-cache footprint.
- For latency-critical HFT hot paths, **static dispatch** (CRTP/direct functions) is preferred. Keep **virtual** at plugin/routing layers rather than inside per‑tick inner loops.

## Trade-offs
- **CRTP Pros:** zero-overhead polymorphism, aggressive inlining, compile-time composition.
- **CRTP Cons:** code bloat (template instantiations), longer compile times, header/ABI coupling.
- **Virtual Pros:** runtime extensibility, clean modular boundaries, dynamic loading.
- **Virtual Cons:** indirect call overhead, worse inlinability, sometimes poorer code layout.

## Perf Counters (to fill if run on Linux/WSL2)
Commands:
```bash
# pin to one core
taskset -c 0 perf stat -e cycles,instructions,branches,branch-misses ./hft 20000000 1
# (optional) per-variant if you add a RUN_ONLY gate
RUN_ONLY=free    taskset -c 0 perf stat -e cycles,instructions,branches,branch-misses ./hft 20000000 1
RUN_ONLY=virtual taskset -c 0 perf stat -e cycles,instructions,branches,branch-misses ./hft 20000000 1
RUN_ONLY=crtp    taskset -c 0 perf stat -e cycles,instructions,branches,branch-misses ./hft 20000000 1
```

Template table:
| Variant  | cycles | instructions | branches | branch-misses |
|---------|-------:|-------------:|---------:|--------------:|
| free    |        |              |          |               |
| virtual |        |              |          |               |
| crtp    |        |              |          |               |

## Notes
- Results vary with CPU/OS/compiler/flags and power/thermal state; pinning and “High performance” plan reduce variance.
- SoA can help vectorization and cache behavior but benefits depend on compiler and microarchitecture.
- “Optimization” ideas (e.g., reciprocal) must be validated by measurement; they are not universally faster.
