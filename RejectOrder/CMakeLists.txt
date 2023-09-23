set(HEADER_FILE RejectOrder.h)
set(SOURCE_FILES RejectOrder.cpp)

add_library(RejectOrder STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(RejectOrder PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link the base class library
target_link_libraries(RejectOrder OrderCore)

# You can link additional libraries if needed
# target_link_libraries(derived_class some_other_library)