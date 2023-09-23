set(HEADER_FILES OrderCore.h)
set(SOURCE_FILES OrderCore.cpp)

add_library(OrderCore STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(OrderCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# You can link additional libraries if needed
# target_link_libraries(base_class some_other_library)