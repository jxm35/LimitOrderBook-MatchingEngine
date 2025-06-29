add_subdirectory(simulation)

add_executable(exchange exchange/main.cpp)
target_link_libraries(exchange
        OrderBook
        MDFeed
        Order
        OrderBookEntry
        Security
        MatchResult
)

add_executable(client client/main.cpp)
target_link_libraries(client MDFeed)
