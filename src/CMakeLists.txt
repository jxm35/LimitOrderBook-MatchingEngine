add_executable(exchange exchange/main.cpp)
target_link_libraries(exchange OrderBook MDFeed)

add_executable(client client/main.cpp)
target_link_libraries(client MDFeed)

add_executable(order_server order_server/main.cpp)
target_link_libraries(order_server OrderEntry)

add_executable(order_client order_client/main.cpp)
target_link_libraries(order_client OrderEntry)
