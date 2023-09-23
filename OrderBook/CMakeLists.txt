set(HEADER_FILES OrderBook.h)
set(SOURCE_FILES OrderBook.cpp)

add_library(OrderBook STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(OrderBook PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(OrderBook Order OrderBookEntry MatchResult Security)

# You can link additional libraries if needed
# target_link_libraries(base_class some_other_library)