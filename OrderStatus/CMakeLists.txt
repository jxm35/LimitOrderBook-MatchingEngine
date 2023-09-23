set(HEADER_FILE OrderStatus.h)
set(SOURCE_FILES OrderStatus.cpp)

add_library(OrderStatus STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(OrderStatus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link the base class library
#target_link_libraries(RejectOrder OrderCore)

# You can link additional libraries if needed
# target_link_libraries(derived_class some_other_library)