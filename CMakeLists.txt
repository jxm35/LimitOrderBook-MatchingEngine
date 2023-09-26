cmake_minimum_required(VERSION 3.24)
project(LimitOrderBook)

set(CMAKE_CXX_STANDARD 20)

# Boost setup
set(BOOST_ROOT /opt/homebrew/Cellar/boost/1.82.0_1)
set(BOOST_INCLUDE_DIR /opt/homebrew/Cellar/boost/1.82.0_1/include)
set(BOOST_LIBRARY_DIR /opt/homebrew/Cellar/boost/1.82.0_1/lib)

find_package(Boost 1.82.0 COMPONENTS REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
if (NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif ()


# Spdlog / Fmt setup
set(SPDLOG_ROOT /opt/homebrew/Cellar/spdlog/1.12.0)
set(FMT_ROOT /opt/homebrew/Cellar/fmt/10.1.1)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
include_directories(/opt/homebrew/Cellar/spdlog/1.12.0/include)
include_directories(/opt/homebrew/Cellar/fmt/10.1.1/include)

add_subdirectory(OrderCore)
add_subdirectory(Order)
add_subdirectory(CancelOrder)
add_subdirectory(OrderStatusResolver)
add_subdirectory(RejectOrder)
add_subdirectory(RejectionResolver)
add_subdirectory(OrderBookEntry)
add_subdirectory(Limit)
add_subdirectory(OrderBook)
add_subdirectory(MatchResult)
add_subdirectory(Security)
add_subdirectory(OrderStatus)

add_subdirectory(OrderBookTests)

target_link_libraries(OrderCore Order CancelOrder RejectOrder OrderStatusResolver RejectionResolver OrderBookEntry Limit OrderBook MatchResult Security OrderStatus)


#if(NOT SPDLOG_FOUND)
#    message(FATAL_ERROR "Could not find spdlog!")
#endif()

add_subdirectory(server)
#add_subdirectory(client)

add_executable(WebSocketClient client/main.cpp client/session.cpp client/util.cpp)
target_link_libraries(WebSocketClient PRIVATE)

add_executable(RunOrderBook main.cpp)
target_link_libraries(RunOrderBook PRIVATE OrderBook)

add_subdirectory(realtime)


#add_executable(networking server/main.cpp)

#add_executable(client client/main.cpp)
