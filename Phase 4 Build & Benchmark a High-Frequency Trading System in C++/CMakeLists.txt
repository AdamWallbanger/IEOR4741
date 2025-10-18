cmake_minimum_required(VERSION 3.10)
project(HFT_Project)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(hft_app
        src/main.cpp
        src/MarketData.cpp
        src/MatchingEngine.cpp
        src/OrderManager.cpp
        src/TradeLogger.cpp
)