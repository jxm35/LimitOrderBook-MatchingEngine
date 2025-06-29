#include <iostream>
#include <csignal>
#include <memory>

#include "receiver/MulticastReceiver.h"

namespace
{
    std::unique_ptr<mdfeed::MulticastReceiver> receiver;

    void signal_handler(const int signal)
    {
        std::cout << "\nReceived signal " << signal << ", shutting down client..." << std::endl;
        if (receiver)
        {
            receiver->stop();
        }
    }
}

class MarketDataClient
{
private:
    std::unique_ptr<mdfeed::MulticastReceiver> receiver_;

public:
    explicit MarketDataClient(const mdfeed::ReceiverConfig& config)
        : receiver_(std::make_unique<mdfeed::MulticastReceiver>(config))
    {
        receiver_->set_message_handler(
            [this](const mdfeed::MessageHeader& header, const void* data, const size_t length)
            {
                process_market_data(header, data, length);
            });
    }

    bool start() const
    {
        std::cout << "Starting Market Data Client..." << std::endl;
        return receiver_->start();
    }

    void stop() const
    {
        std::cout << "Stopping Market Data Client..." << std::endl;
        if (receiver_)
        {
            receiver_->stop();
        }
    }

    [[nodiscard]] bool is_running() const
    {
        return receiver_ && receiver_->is_running();
    }

    [[nodiscard]] mdfeed::MulticastReceiver::Stats get_stats() const
    {
        return receiver_ ? receiver_->get_stats() : mdfeed::MulticastReceiver::Stats{};
    }

private:
    static void process_market_data(const mdfeed::MessageHeader& header, const void* data, size_t length)
    {
        (void)header;
        (void)data;
        (void)length;
        // TODO: Implement
    }
};

int main(int argc, char* argv[])
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    mdfeed::ReceiverConfig config;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--ip" && i + 1 < argc)
        {
            config.multicast_ip = argv[++i];
        }
        else if (arg == "--port" && i + 1 < argc)
        {
            config.multicast_port = static_cast<uint16_t>(std::stoul(argv[++i]));
        }
        else if (arg == "--interface" && i + 1 < argc)
        {
            config.interface_ip = argv[++i];
        }
        else if (arg == "--logfile" && i + 1 < argc)
        {
            config.log_file_path = argv[++i];
            config.log_to_console = false; // Only log to file if specified
        }
        else if (arg == "--no-console")
        {
            config.log_to_console = false;
        }
        else if (arg == "--no-validation")
        {
            config.validate_sequence_numbers = false;
        }
        else if (arg == "--stats-interval" && i + 1 < argc)
        {
            config.stats_interval = std::chrono::milliseconds(std::stoul(argv[++i]));
        }
        else if (arg == "--help" || arg == "-h")
        {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                << "Options:\n"
                << "  --ip <address>          Multicast IP address (default: " << config.multicast_ip << ")\n"
                << "  --port <port>           Multicast port (default: " << config.multicast_port << ")\n"
                << "  --interface <ip>        Local interface IP (default: " << config.interface_ip << ")\n"
                << "  --logfile <path>        Log to file instead of console\n"
                << "  --no-console            Disable console logging\n"
                << "  --no-validation         Disable sequence number validation\n"
                << "  --stats-interval <ms>   Statistics interval in ms (default: 5000)\n"
                << "  --help, -h              Show this help message\n";
            return 0;
        }
        else
        {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }

    std::cout << "=== MARKET DATA CLIENT ===" << std::endl;
    std::cout << "Multicast Address: " << config.multicast_ip << ":" << config.multicast_port << std::endl;
    std::cout << "Interface: " << config.interface_ip << std::endl;

    if (!config.log_file_path.empty())
    {
        std::cout << "Logging to file: " << config.log_file_path << std::endl;
    }

    std::cout << "Sequence validation: " << (config.validate_sequence_numbers ? "enabled" : "disabled") << std::endl;
    std::cout << "Press Ctrl+C to stop..." << std::endl;

    try
    {
        MarketDataClient client(config);
        receiver = std::make_unique<mdfeed::MulticastReceiver>(config);

        if (!client.start())
        {
            std::cerr << "Failed to start market data client!" << std::endl;
            return 1;
        }

        while (client.is_running())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        client.stop();

        auto stats = client.get_stats();
        std::cout << "\n=== FINAL STATISTICS ===" << std::endl;
        std::cout << "Total Messages: " << stats.total_messages_received << std::endl;
        std::cout << "Total Bytes: " << stats.total_bytes_received << std::endl;
        std::cout << "Sequence Gaps: " << stats.sequence_gaps << std::endl;
        std::cout << "Invalid Messages: " << stats.invalid_messages << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Client error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Market data client stopped." << std::endl;
    return 0;
}
