#include "messages/OrderMessages.h"
#include "server/OrderEntryServer.h"
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

std::unique_ptr<orderentry::OrderEntryServer> server;

void signal_handler(int signal)
{
    std::cout << "\nShutting down server..." << signal << std::endl;
    if (server) { server->stop(); }
}

int main(int argc, char* argv[])
{
    std::signal(SIGINT, signal_handler);

    uint16_t port = 8080;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = static_cast<uint16_t>(std::stoul(argv[++i]));
        }
        else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [--port <port>]"
                      << std::endl;
            std::cout << "Default port: 8080" << std::endl;
            return 0;
        }
    }

    server = std::make_unique<orderentry::OrderEntryServer>(port);

    if (!server->start()) {
        std::cerr << "Failed to start server on port " << port << std::endl;
        return 1;
    }

    auto* order_buffer = server->get_order_buffer();
    orderentry::OrderBuffer order;

    std::cout << "=== ORDER ENTRY SERVER ===" << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "Press Ctrl+C to stop..." << std::endl;
    std::cout << "Waiting for orders..." << std::endl;

    uint64_t processed_orders = 0;

    while (server->is_running()) {
        if (order_buffer->pop(order)) {
            processed_orders++;

            const auto* header
                    = reinterpret_cast<const orderentry::MessageHeader*>(
                            order.data);
            if (static_cast<orderentry::MessageType>(header->message_type)
                == orderentry::MessageType::NEW_ORDER) {
                const auto* order_msg = order.as<orderentry::NewOrderMessage>();

                std::cout << "Order " << processed_orders << ": "
                          << order_msg->client_order_id << " "
                          << std::string(order_msg->symbol, 8) << " "
                          << (order_msg->side == orderentry::Side::BUY ? "BUY"
                                                                       : "SELL")
                          << " " << order_msg->quantity << " @ "
                          << (order_msg->price / 100.0) << std::endl;
            }
            else if (static_cast<orderentry::MessageType>(header->message_type)
                     == orderentry::MessageType::CANCEL_ORDER) {
                const auto* cancel_msg
                        = order.as<orderentry::CancelOrderMessage>();

                std::cout << "Cancel " << processed_orders << ": "
                          << cancel_msg->client_order_id << " (original: "
                          << cancel_msg->original_client_order_id << ")"
                          << " " << std::string(cancel_msg->symbol, 8)
                          << std::endl;
            }
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    auto stats = server->get_stats();
    std::cout << "\n=== FINAL STATS ===" << std::endl;
    std::cout << "Processed orders: " << processed_orders << std::endl;
    std::cout << "Received orders: " << stats.orders_received << std::endl;
    std::cout << "Total connections: " << stats.connections << std::endl;
    std::cout << "Server stopped." << std::endl;

    return 0;
}
