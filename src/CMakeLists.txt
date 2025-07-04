add_executable(exchange exchange/main.cpp)
target_link_libraries(exchange OrderBook MDFeed)

add_executable(client client/main.cpp)
target_link_libraries(client MDFeed)
