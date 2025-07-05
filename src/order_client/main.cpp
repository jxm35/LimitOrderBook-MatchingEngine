#include "client/OrderEntryClient.h"
#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>

void print_usage(const char* program_name)
{
    std::cout << "Usage: " << program_name << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --server <address>    Server address (default: 127.0.0.1)"
              << std::endl;
    std::cout << "  --port <port>         Server port (default: 8080)"
              << std::endl;
    std::cout << "  --help, -h            Show this help message" << std::endl;
}

void interactive_mode(orderentry::OrderEntryClient& client)
{
    std::cout << "\n=== INTERACTIVE ORDER ENTRY CLIENT ===" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  new <symbol> <side> <price> <quantity>  - Send new order "
                 "(side: buy/sell)"
              << std::endl;
    std::cout << "  cancel <order_id> <symbol>              - Cancel order"
              << std::endl;
    std::cout << "  quit                                     - Exit"
              << std::endl;
    std::cout << "=======================================" << std::endl;

    uint64_t next_order_id = 1;
    std::string line;

    std::cout << "> ";
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "quit" || command == "q" || command == "exit") { break; }
        else if (command == "new") {
            std::string symbol, side_str;
            double price;
            uint64_t quantity;

            if (!(iss >> symbol >> side_str >> price >> quantity)) {
                std::cout << "Usage: new <symbol> <side> <price> <quantity>"
                          << std::endl;
                std::cout << "> ";
                continue;
            }

            orderentry::Side side = (side_str == "buy" || side_str == "BUY")
                                            ? orderentry::Side::BUY
                                            : orderentry::Side::SELL;

            uint64_t price_ticks
                    = static_cast<uint64_t>(price * 100); // Convert to pence

            if (client.send_new_order(next_order_id, symbol, side, price_ticks,
                                      quantity)) {
                std::cout << "Sent NEW_ORDER: id=" << next_order_id
                          << ", symbol=" << symbol << ", side=" << side_str
                          << ", price=" << price << ", qty=" << quantity
                          << std::endl;
                next_order_id++;
            }
            else {
                std::cout << "Failed to send order" << std::endl;
            }
        }
        else if (command == "cancel") {
            std::string symbol;
            uint64_t order_id;

            if (!(iss >> order_id >> symbol)) {
                std::cout << "Usage: cancel <order_id> <symbol>" << std::endl;
                std::cout << "> ";
                continue;
            }

            if (client.send_cancel_order(next_order_id, order_id, symbol)) {
                std::cout << "Sent CANCEL_ORDER: cancel_id=" << next_order_id
                          << ", original_id=" << order_id
                          << ", symbol=" << symbol << std::endl;
                next_order_id++;
            }
            else {
                std::cout << "Failed to send cancel" << std::endl;
            }
        }
        else if (command == "help") {
            std::cout << "Commands:" << std::endl;
            std::cout << "  new <symbol> <side> <price> <quantity>"
                      << std::endl;
            std::cout << "  cancel <order_id> <symbol>" << std::endl;
            std::cout << "  quit" << std::endl;
        }
        else if (!command.empty()) {
            std::cout << "Unknown command: " << command
                      << ". Type 'help' for commands." << std::endl;
        }

        std::cout << "> ";
    }
}

int main(int argc, char* argv[])
{
    std::string server_address = "127.0.0.1";
    uint16_t port = 8080;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--server" && i + 1 < argc) { server_address = argv[++i]; }
        else if (arg == "--port" && i + 1 < argc) {
            port = static_cast<uint16_t>(std::stoul(argv[++i]));
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

    std::cout << "=== ORDER ENTRY CLIENT ===" << std::endl;
    std::cout << "Server: " << server_address << ":" << port << std::endl;

    orderentry::OrderEntryClient client;

    if (!client.connect(server_address, port)) {
        std::cerr << "Failed to connect to server!" << std::endl;
        std::cerr << "Make sure the server is running: ./order_server --port "
                  << port << std::endl;
        return 1;
    }

    try {
        interactive_mode(client);
    }
    catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
        return 1;
    }

    client.disconnect();
    std::cout << "Order entry client finished." << std::endl;
    return 0;
}
