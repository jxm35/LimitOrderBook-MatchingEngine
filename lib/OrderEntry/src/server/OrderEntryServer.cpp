#include "server/OrderEntryServer.h"
#include "messages/OrderMessages.h"
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace orderentry {
    OrderEntryServer::OrderEntryServer(uint16_t port)
        : port_(port), server_fd_(-1),
          order_buffer_(std::make_unique<OrderRingBuffer>(65536)),
          running_(false)
    {}

    OrderEntryServer::~OrderEntryServer()
    {
        stop();
    }

    bool OrderEntryServer::start()
    {
        if (running_.load()) return true;

        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        int opt = 1;
        setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);

        if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Bind failed on port " << port_ << std::endl;
            close(server_fd_);
            return false;
        }

        if (listen(server_fd_, 10) < 0) {
            std::cerr << "Listen failed" << std::endl;
            close(server_fd_);
            return false;
        }

        running_.store(true);
        server_thread_ = std::thread(&OrderEntryServer::server_loop, this);

        std::cout << "OrderEntry server started on port " << port_ << std::endl;
        return true;
    }

    void OrderEntryServer::stop()
    {
        if (!running_.load()) return;

        running_.store(false);

        if (server_fd_ >= 0) {
            close(server_fd_);
            server_fd_ = -1;
        }

        if (server_thread_.joinable()) { server_thread_.join(); }

        std::cout << "OrderEntry server stopped" << std::endl;
    }

    void OrderEntryServer::server_loop()
    {
        while (running_.load()) {
            sockaddr_in client_address{};
            socklen_t client_len = sizeof(client_address);

            int client_fd = accept(
                    server_fd_, (struct sockaddr*)&client_address, &client_len);
            if (client_fd < 0) {
                if (running_.load()) {
                    std::cerr << "Accept failed" << std::endl;
                }
                continue;
            }

            stats_.connections++;
            std::cout << "New client connected (fd=" << client_fd << ")"
                      << std::endl;

            std::thread client_thread(&OrderEntryServer::handle_client, this,
                                      client_fd);
            client_thread.detach();
        }
    }

    void OrderEntryServer::handle_client(int client_fd)
    {
        char buffer[1024];

        while (running_.load()) {
            ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);

            if (bytes_read <= 0) {
                break; // Client disconnected
            }

            size_t offset = 0;
            while (offset + sizeof(MessageHeader)
                   <= static_cast<size_t>(bytes_read)) {
                const auto* header = reinterpret_cast<const MessageHeader*>(
                        buffer + offset);

                if (offset + header->message_length
                    <= static_cast<size_t>(bytes_read)) {
                    OrderBuffer order_buf;
                    if (header->message_length
                        <= OrderBuffer::MAX_MESSAGE_SIZE) {
                        std::memcpy(order_buf.data, buffer + offset,
                                    header->message_length);
                        order_buf.length = header->message_length;
                        order_buf.client_fd = client_fd;

                        if (order_buffer_->push(order_buf)) {
                            stats_.orders_received++;

                            if (static_cast<MessageType>(header->message_type)
                                == MessageType::NEW_ORDER) {
                                const auto* order = reinterpret_cast<
                                        const NewOrderMessage*>(buffer
                                                                + offset);
                                std::cout
                                        << "Received order: id="
                                        << order->client_order_id << ", symbol="
                                        << std::string(order->symbol, 8)
                                        << ", side="
                                        << (order->side == Side::BUY ? "BUY"
                                                                     : "SELL")
                                        << ", price=" << (order->price / 100.0)
                                        << ", qty=" << order->quantity
                                        << std::endl;
                            }
                        }
                    }

                    offset += header->message_length;
                }
                else {
                    break; // Incomplete message
                }
            }
        }

        close(client_fd);
        std::cout << "Client disconnected (fd=" << client_fd << ")"
                  << std::endl;
    }
} // namespace orderentry
