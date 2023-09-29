add_subdirectory(Limit)
add_subdirectory(MatchResult)
add_subdirectory(Order)
add_subdirectory(OrderBook)
add_subdirectory(OrderBookEntry)
add_subdirectory(OrderStatus)
add_subdirectory(OrderStatusResolver)
add_subdirectory(RejectionResolver)
add_subdirectory(Security)
#add_library(lib STATIC ${SOURCE_FILES} ${HEADER_FILES})

#target_include_directories(lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link the base class library
#target_link_libraries(lib Limit MatchResult Order OrderBook OrderBookEntry OrderStatus OrderStatusResolver RejectionResolver Security)