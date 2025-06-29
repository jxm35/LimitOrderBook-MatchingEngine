#pragma once

#include "MulticastPublisher.h"
#include "utils/RingBuffer.h"
#include "utils/PublisherConfig.h"
#include <thread>
#include <atomic>

namespace mdfeed
{
    class MulticastPublisherThread
    {
    public:
        explicit MulticastPublisherThread(MDRingBuffer* ring_buffer,
                                          const PublisherConfig& config = PublisherConfig{});
        ~MulticastPublisherThread();

        MulticastPublisherThread(const MulticastPublisherThread&) = delete;
        MulticastPublisherThread& operator=(const MulticastPublisherThread&) = delete;

        bool start();
        void stop();
        bool is_running() const { return running_.load(); }

        struct Stats
        {
            uint64_t messages_sent = 0;
            uint64_t send_failures = 0;
            uint64_t ring_buffer_empty_count = 0;
        };

        Stats get_stats() const { return stats_; }

    private:
        void publisher_loop();

        MDRingBuffer* ring_buffer_;
        PublisherConfig config_;
        MulticastPublisher multicast_publisher_;
        std::atomic<bool> running_;
        std::thread publisher_thread_;
        Stats stats_;
    };
}
