#include "Exchange.h"
#include "utils/PublisherConfig.h"
#include <csignal>
#include <iostream>
#include <memory>

namespace {
    std::unique_ptr<Exchange> exchange;

    void signal_handler(const int signal)
    {
        std::cout << "\nReceived signal " << signal
                  << ", shutting down exchange..." << std::endl;
        if (exchange) { exchange->stop(); }
        exit(0);
    }
} // namespace

void print_usage(const char* program_name)
{
    std::cout << "Usage: " << program_name << " [options]\n";
    std::cout << "Options:\n";
    std::cout << "  --mode <mode>            Exchange mode: 'simulation' or "
                 "'client' (default: simulation)\n";
    std::cout << "  --md-ip <address>        Market data multicast IP "
                 "(default: 239.1.1.1)\n";
    std::cout << "  --md-port <port>         Market data multicast port "
                 "(default: 9999)\n";
    std::cout << "  --md-interface <ip>      Market data interface IP "
                 "(default: 127.0.0.1)\n";
    std::cout << "  --oe-port <port>         Order entry TCP port (default: "
                 "8080, client mode only)\n";
    std::cout << "  --help, -h               Show this help message\n";
    std::cout << "\nModes:\n";
    std::cout << "  simulation              Generate synthetic trading "
                 "activity\n";
    std::cout << "  client                  Accept orders from TCP clients\n";
    std::cout << "\nSupported Symbols: AAPL, GOOGL, AMZN, NFLX, META\n";
}

int main(const int argc, char* argv[])
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    auto mode = Exchange::Mode::SIMULATION;
    mdfeed::PublisherConfig md_config;
    uint16_t oe_port = 8080;

    for (int i = 1; i < argc; ++i) {
        if (std::string arg = argv[i]; arg == "--mode" && i + 1 < argc) {
            if (std::string mode_str = argv[++i]; mode_str == "simulation") {
                mode = Exchange::Mode::SIMULATION;
            }
            else if (mode_str == "client") {
                mode = Exchange::Mode::CLIENT_ORDERS;
            }
            else {
                std::cerr << "Invalid mode: " << mode_str << std::endl;
                print_usage(argv[0]);
                return 1;
            }
        }
        else if (arg == "--md-ip" && i + 1 < argc) {
            md_config.multicast_ip = argv[++i];
        }
        else if (arg == "--md-port" && i + 1 < argc) {
            md_config.multicast_port
                    = static_cast<uint16_t>(std::stoul(argv[++i]));
        }
        else if (arg == "--md-interface" && i + 1 < argc) {
            md_config.interface_ip = argv[++i];
        }
        else if (arg == "--oe-port" && i + 1 < argc) {
            oe_port = static_cast<uint16_t>(std::stoul(argv[++i]));
        }
        else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }

    std::cout << "=== ENHANCED EXCHANGE ===" << std::endl;
    std::cout << "Mode: "
              << (mode == Exchange::Mode::SIMULATION ? "SIMULATION"
                                                     : "CLIENT_ORDERS")
              << std::endl;
    std::cout << "Market Data: " << md_config.multicast_ip << ":"
              << md_config.multicast_port << std::endl;
    std::cout << "MD Interface: " << md_config.interface_ip << std::endl;
    if (mode == Exchange::Mode::CLIENT_ORDERS) {
        std::cout << "Order Entry Port: " << oe_port << std::endl;
    }

    std::cout << "Press Ctrl+C to stop..." << std::endl;

    try {
        exchange = std::make_unique<Exchange>(mode, md_config, oe_port);
        exchange->run();
    }
    catch (const std::exception& e) {
        std::cerr << "Exchange error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Enhanced exchange stopped." << std::endl;
    return 0;
}
