set(HEADER_FILES CancelOrder.h)
set(SOURCE_FILES CancelOrder.cpp)

add_library(CancelOrder STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(CancelOrder PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link the base class library
target_link_libraries(CancelOrder OrderCore)

# You can link additional libraries if needed
# target_link_libraries(derived_class some_other_library)