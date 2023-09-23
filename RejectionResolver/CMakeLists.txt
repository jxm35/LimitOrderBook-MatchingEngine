set(HEADER_FILE RejectionResolver.h)
set(SOURCE_FILES RejectionResolver.cpp)

add_library(RejectionResolver STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(RejectionResolver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link the base class library
target_link_libraries(RejectionResolver RejectOrder OrderCore)