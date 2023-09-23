set(HEADER_FILES Security.h)
set(SOURCE_FILES Security.cpp)

add_library(Security STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(Security PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# You can link additional libraries if needed
# target_link_libraries(base_class some_other_library)