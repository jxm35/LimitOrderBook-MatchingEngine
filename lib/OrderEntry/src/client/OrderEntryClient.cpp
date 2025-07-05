#include "client/OrderEntryClient.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace orderentry {
    OrderEntryClient::OrderEntryClient(): socket_fd_(-1), sequence_number_(1) {}

    OrderEntryClient::~OrderEntryClient()
    {
        disconnect();
    }

    bool OrderEntryClient::connect(const std::string& server_address,
                                   uint16_t port)
    {
        if (is_connected()) return true;

        socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd_ < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, server_address.c_str(), &server_addr.sin_addr)
            <= 0) {
            std::cerr << "Invalid address: " << server_address << std::endl;
            close(socket_fd_);
            socket_fd_ = -1;
            return false;
        }

        if (::connect(socket_fd_, (struct sockaddr*)&server_addr,
                      sizeof(server_addr))
            < 0) {
            std::cerr << "Connection failed to " << server_address << ":"
                      << port << std::endl;
            close(socket_fd_);
            socket_fd_ = -1;
            return false;
        }

        std::cout << "Connected to " << server_address << ":" << port
                  << std::endl;
        return true;
    }

    void OrderEntryClient::disconnect()
    {
        if (socket_fd_ >= 0) {
            close(socket_fd_);
            socket_fd_ = -1;
            std::cout << "Disconnected from server" << std::endl;
        }
    }

    bool OrderEntryClient::send_new_order(uint64_t client_order_id,
                                          const std::string& symbol, Side side,
                                          uint64_t price, uint64_t quantity)
    {
        if (!is_connected()) return false;

        NewOrderMessage msg{};
        message_utils::init_header(msg, MessageType::NEW_ORDER,
                                   sequence_number_++, 1);
        msg.client_order_id = client_order_id;
        msg.price = price;
        msg.quantity = quantity;
        msg.side = side;
        msg.order_type = OrderType::LIMIT;
        msg.time_in_force = TimeInForce::DAY;

        size_t copy_len = std::min(symbol.length(), sizeof(msg.symbol));
        std::memcpy(msg.symbol, symbol.c_str(), copy_len);
        if (copy_len < sizeof(msg.symbol)) {
            std::memset(msg.symbol + copy_len, 0,
                        sizeof(msg.symbol) - copy_len);
        }

        std::memset(msg.reserved, 0, sizeof(msg.reserved));

        return send_message(&msg, sizeof(msg));
    }

    bool OrderEntryClient::send_cancel_order(uint64_t client_order_id,
                                             uint64_t original_order_id,
                                             const std::string& symbol)
    {
        if (!is_connected()) return false;

        CancelOrderMessage msg{};
        message_utils::init_header(msg, MessageType::CANCEL_ORDER,
                                   sequence_number_++, 1);
        msg.client_order_id = client_order_id;
        msg.original_client_order_id = original_order_id;

        size_t copy_len = std::min(symbol.length(), sizeof(msg.symbol));
        std::memcpy(msg.symbol, symbol.c_str(), copy_len);
        if (copy_len < sizeof(msg.symbol)) {
            std::memset(msg.symbol + copy_len, 0,
                        sizeof(msg.symbol) - copy_len);
        }

        std::memset(msg.reserved, 0, sizeof(msg.reserved));

        return send_message(&msg, sizeof(msg));
    }

    bool OrderEntryClient::send_message(const void* data, size_t length)
    {
        if (!is_connected()) return false;

        ssize_t bytes_sent = send(socket_fd_, data, length, 0);
        if (bytes_sent < 0) {
            std::cerr << "Failed to send message" << std::endl;
            disconnect();
            return false;
        }

        return bytes_sent == static_cast<ssize_t>(length);
    }
} // namespace orderentry
