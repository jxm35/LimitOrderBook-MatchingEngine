#include "publisher/MulticastPublisherThread.h"
#include <iostream>
#include <chrono>

namespace mdfeed
{
    MulticastPublisherThread::MulticastPublisherThread(MDRingBuffer* ring_buffer, const PublisherConfig& config)
        : ring_buffer_(ring_buffer)
          , config_(config)
          , running_(false)
    {
    }

    MulticastPublisherThread::~MulticastPublisherThread()
    {
        stop();
    }

    bool MulticastPublisherThread::start()
    {
        if (running_.load())
        {
            return true;
        }

        if (!ring_buffer_)
        {
            std::cerr << "No ring buffer provided to MulticastPublisherThread!" << std::endl;
            return false;
        }
        if (!multicast_publisher_.initialize(config_.multicast_ip,
                                             config_.multicast_port,
                                             config_.interface_ip))
        {
            std::cerr << "Failed to initialize multicast publisher!" << std::endl;
            return false;
        }

        running_.store(true);
        stats_ = Stats{};

        publisher_thread_ = std::thread(&MulticastPublisherThread::publisher_loop, this);

        std::cout << "MulticastPublisherThread started - publishing to "
            << config_.multicast_ip << ":" << config_.multicast_port << std::endl;
        return true;
    }

    void MulticastPublisherThread::stop()
    {
        if (!running_.load())
        {
            return;
        }

        std::cout << "Stopping MulticastPublisherThread..." << std::endl;
        running_.store(false);

        if (publisher_thread_.joinable())
        {
            publisher_thread_.join();
        }

        multicast_publisher_.close();

        std::cout << "MulticastPublisherThread stopped. Messages sent: "
            << stats_.messages_sent << ", Failures: " << stats_.send_failures << std::endl;
    }

    void MulticastPublisherThread::publisher_loop()
    {
        MessageBuffer message;
        auto last_stats_time = std::chrono::steady_clock::now();
        std::cout << "Publisher thread started" << std::endl;
        while (running_.load(std::memory_order_relaxed))
        {
            if (ring_buffer_->pop(message))
            {
                if (multicast_publisher_.send(message.data, message.length))
                {
                    stats_.messages_sent++;

                    auto now = std::chrono::steady_clock::now();
                    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_stats_time).count() >= 5)
                    {
                        std::cout << "Publisher stats - Sent: " << stats_.messages_sent
                            << ", Failures: " << stats_.send_failures
                            << ", Empty polls: " << stats_.ring_buffer_empty_count << std::endl;
                        last_stats_time = now;
                    }
                }
                else
                {
                    stats_.send_failures++;
                    std::cerr << "Failed to send multicast message" << std::endl;
                }
            }
            else
            {
                stats_.ring_buffer_empty_count++;

                if (config_.use_busy_wait)
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    while (std::chrono::high_resolution_clock::now() - start < config_.spin_duration)
                    {
                    }
                }
                else
                {
                    // Yield to other threads
                    std::this_thread::yield();
                }
            }
        }

        std::cout << "Publisher thread stopped." << std::endl;
    }
}
