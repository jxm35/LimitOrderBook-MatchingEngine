set(HEADER_FILES
        include/core/OrderBook.h
        include/entries/OrderBookEntry.h
        include/orders/Order.h
        include/orders/OrderCore.h
        include/securities/Security.h
        include/status/OrderStatus.h
        )

set(SOURCE_FILES
        src/core/OrderBook.cpp
        src/entries/OrderBookEntry.cpp
        src/orders/Order.cpp
        src/orders/OrderCore.cpp
        src/securities/Security.cpp
        )

add_library(OrderBook STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(OrderBook PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(OrderBook MDFeed spdlog::spdlog)
