set(HEADER_FILE OrderStatusResolver.h)
set(SOURCE_FILES OrderStatusResolver.cpp)

add_library(OrderStatusResolver STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(OrderStatusResolver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link the base class library
target_link_libraries(OrderStatusResolver Order OrderStatus)

# You can link additional libraries if needed
# target_link_libraries(derived_class some_other_library)