set(HEADER_FILE OrderCore.h order.h CancelOrder.h RejectOrder.h)
set(SOURCE_FILES OrderCore.cpp order.cpp CancelOrder.cpp RejectOrder.cpp)

add_library(Order STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(Order PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link the base class library
target_link_libraries(Order fmt::fmt)

# You can link additional libraries if needed
# target_link_libraries(derived_class some_other_library)