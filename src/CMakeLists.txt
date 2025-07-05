add_executable(exchange
        exchange/Exchange.cpp
        exchange/SymbolManager.cpp
        exchange/main.cpp
)

target_include_directories(exchange PRIVATE exchange)
target_link_libraries(exchange OrderBook MDFeed OrderEntry)

add_executable(client client/main.cpp)
target_link_libraries(client MDFeed)

add_executable(order_server order_server/main.cpp)
target_link_libraries(order_server OrderEntry)

add_executable(order_client order_client/main.cpp)
target_link_libraries(order_client OrderEntry)
