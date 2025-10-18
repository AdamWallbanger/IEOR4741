cmake_minimum_required(VERSION 3.16)
project(HighPerfOrderBook CXX)

# Standard & Release flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# LTO (IPO) if your toolchain supports it
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_ok OUTPUT ipo_err)
if(ipo_ok)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Headers live in the project root (adjust if you moved them to include/)
include_directories(${CMAKE_SOURCE_DIR})

# ---------------------------------------------------------
# Common sources WITH NO main()
# Heaps (final) implementation has .cpp + .h
set(SRC_HEAPS order_book.cpp)

# Map & Vector variants are header-only in this setup
# (If you actually have .cpp files for them, add them here)
set(HDR_MAP     order_book_map.h)
set(HDR_VECTOR  order_book_vector.h)

# ---------------------------------------------------------
# Unit tests (pick the implementation you want to test)
add_executable(test_order_book
        ${SRC_HEAPS}           # uses the heaps impl
        test_order_book.cpp
)

# ---------------------------------------------------------
# Benchmarks
# 1) Final STL + Heaps (default)
add_executable(benchmark_heaps
        ${SRC_HEAPS}
        benchmark.cpp
)
# No macro needed; benchmark.cpp includes "order_book.h" by default

# 2) HashMap + std::map (no heaps) — header-only build
add_executable(benchmark_map
        benchmark.cpp         # only the bench; impl picked via macro
        ${HDR_MAP}
)
target_compile_definitions(benchmark_map PRIVATE USE_MAP_BOOK)

# 3) Baseline Vector — header-only build
add_executable(benchmark_vector
        benchmark.cpp
        ${HDR_VECTOR}
)
target_compile_definitions(benchmark_vector PRIVATE USE_VECTOR_BOOK)

# Nice target names in CLion
set_target_properties(test_order_book  PROPERTIES OUTPUT_NAME "test_order_book")
set_target_properties(benchmark_heaps  PROPERTIES OUTPUT_NAME "benchmark_heaps")
set_target_properties(benchmark_map    PROPERTIES OUTPUT_NAME "benchmark_map")
set_target_properties(benchmark_vector PROPERTIES OUTPUT_NAME "benchmark_vector")
