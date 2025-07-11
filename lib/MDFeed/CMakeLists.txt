set(HEADER_FILES
        include/messages/Messages.h
        include/utils/RingBuffer.h
        include/utils/PublisherConfig.h
        include/publisher/MulticastPublisher.h
        include/publisher/MarketDataPublisher.h
        include/publisher/MDAdapter.h
        include/publisher/MulticastPublisherThread.h
        include/receiver/ReceiverConfig.h
        include/receiver/MulticastReceiver.h
)

set(SOURCE_FILES
        src/publisher/MulticastPublisher.cpp
        src/publisher/MarketDataPublisher.cpp
        src/publisher/MulticastPublisherThread.cpp
        src/receiver/MulticastReceiver.cpp
)

add_library(MDFeed STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(MDFeed PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Threads REQUIRED)
target_link_libraries(MDFeed Threads::Threads)
