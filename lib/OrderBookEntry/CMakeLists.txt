set(HEADER_FILE OrderBookEntry.h)
set(SOURCE_FILES OrderBookEntry.cpp)

add_library(OrderBookEntry STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(OrderBookEntry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link the base class library
target_link_libraries(OrderBookEntry Order)

# You can link additional libraries if needed
# target_link_libraries(derived_class some_other_library)