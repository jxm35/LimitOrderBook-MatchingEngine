#include "publisher/MulticastPublisher.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <__ostream/print.h>

namespace mdfeed
{
    class MulticastSocket
    {
    public:
        MulticastSocket() : socket_fd_(-1)
        {
        }

        ~MulticastSocket()
        {
            close();
        }

        bool create_and_bind(const std::string& multicast_ip, uint16_t port,
                             const std::string& interface_ip)
        {
            std::println("{}", interface_ip + ":" + std::to_string(port));
            socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
            if (socket_fd_ < 0)
            {
                return false;
            }
            int reuse = 1;
            if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
            {
                close();
                return false;
            }

            std::memset(&multicast_addr_, 0, sizeof(multicast_addr_));
            multicast_addr_.sin_family = AF_INET;
            multicast_addr_.sin_port = htons(port);
            if (inet_aton(multicast_ip.c_str(), &multicast_addr_.sin_addr) == 0)
            {
                close();
                return false;
            }

            struct in_addr local_interface{};
            if (inet_aton(interface_ip.c_str(), &local_interface) == 0)
            {
                close();
                return false;
            }

            if (setsockopt(socket_fd_, IPPROTO_IP, IP_MULTICAST_IF,
                           &local_interface, sizeof(local_interface)) < 0)
            {
                close();
                return false;
            }
            int send_buffer_size = 1024 * 1024; // 1MB send buffer
            setsockopt(socket_fd_, SOL_SOCKET, SO_SNDBUF, &send_buffer_size, sizeof(send_buffer_size));
            return true;
        }

        bool send_data(const void* data, std::size_t length)
        {
            if (socket_fd_ < 0)
            {
                return false;
            }

            ssize_t bytes_sent = sendto(socket_fd_, data, length, 0,
                                        reinterpret_cast<const struct sockaddr*>(&multicast_addr_),
                                        sizeof(multicast_addr_));
            if (bytes_sent < 0)
            {
                std::cerr << "Failed to send data to multicast socket" << std::endl;
                perror("Failed to send data to multicast socket");
            }
            return bytes_sent == static_cast<ssize_t>(length);
        }

        void close()
        {
            if (socket_fd_ >= 0)
            {
                ::close(socket_fd_);
                socket_fd_ = -1;
            }
        }

        bool is_valid() const { return socket_fd_ >= 0; }

    private:
        int socket_fd_;
        struct sockaddr_in multicast_addr_{};
    };

    MulticastPublisher::MulticastPublisher()
        : socket_(std::make_unique<MulticastSocket>()), initialized_(false)
    {
    }

    MulticastPublisher::MulticastPublisher(const std::string& multicast_ip, uint16_t port,
                                           const std::string& interface_ip)
        : socket_(std::make_unique<MulticastSocket>()), initialized_(false)
    {
        initialize(multicast_ip, port, interface_ip);
    }

    MulticastPublisher::~MulticastPublisher()
    {
        close();
    }

    bool MulticastPublisher::initialize(const std::string& multicast_ip, uint16_t port,
                                        const std::string& interface_ip)
    {
        if (initialized_)
        {
            close();
        }
        socket_ = std::make_unique<MulticastSocket>();
        initialized_ = socket_->create_and_bind(multicast_ip, port, interface_ip);

        if (!initialized_)
        {
            socket_.reset();
        }
        return initialized_;
    }

    void MulticastPublisher::close()
    {
        if (socket_)
        {
            socket_->close();
            socket_.reset();
        }
        initialized_ = false;
    }

    bool MulticastPublisher::send(const void* data, std::size_t length)
    {
        if (!initialized_ || !socket_)
        {
            return false;
        }

        return socket_->send_data(data, length);
    }
}
