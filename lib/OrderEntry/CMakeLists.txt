set(HEADER_FILES
        include/messages/OrderMessages.h
        include/utils/OrderBuffer.h
        include/utils/ServerConfig.h
        include/server/OrderEntryServer.h
        include/client/OrderEntryClient.h
)

set(SOURCE_FILES
        src/server/OrderEntryServer.cpp
        src/client/OrderEntryClient.cpp
)

add_library(OrderEntry STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(OrderEntry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Threads REQUIRED)
target_link_libraries(OrderEntry Threads::Threads)
