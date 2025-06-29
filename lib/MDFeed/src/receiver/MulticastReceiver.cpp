#include "receiver/MulticastReceiver.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <iomanip>

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

        bool create_and_join(const std::string& multicast_ip, uint16_t port,
                             const std::string& interface_ip, size_t buffer_size)
        {
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

            int recv_buffer_size = static_cast<int>(buffer_size);
            setsockopt(socket_fd_, SOL_SOCKET, SO_RCVBUF,
                       &recv_buffer_size, sizeof(recv_buffer_size));

            sockaddr_in bind_addr{};
            std::memset(&bind_addr, 0, sizeof(bind_addr));
            bind_addr.sin_family = AF_INET;
            bind_addr.sin_port = htons(port);
            bind_addr.sin_addr.s_addr = INADDR_ANY;

            if (bind(socket_fd_, reinterpret_cast<const sockaddr*>(&bind_addr),
                     sizeof(bind_addr)) < 0)
            {
                close();
                return false;
            }

            ip_mreq mreq{};
            if (inet_aton(multicast_ip.c_str(), &mreq.imr_multiaddr) == 0)
            {
                close();
                return false;
            }

            if (inet_aton(interface_ip.c_str(), &mreq.imr_interface) == 0)
            {
                close();
                return false;
            }

            if (setsockopt(socket_fd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
            {
                close();
                return false;
            }

            return true;
        }

        ssize_t receive_data(void* buffer, size_t buffer_size) const
        {
            if (socket_fd_ < 0)
            {
                return -1;
            }

            return recv(socket_fd_, buffer, buffer_size, 0);
        }

        void close()
        {
            if (socket_fd_ >= 0)
            {
                ::close(socket_fd_);
                socket_fd_ = -1;
            }
        }

        [[nodiscard]] bool is_valid() const { return socket_fd_ >= 0; }

    private:
        int socket_fd_;
    };

    MulticastReceiver::MulticastReceiver()
        : socket_(std::make_unique<MulticastSocket>())
          , running_(false)
          , last_sequence_number_(0)
    {
        stats_.start_time = std::chrono::steady_clock::now();
        last_stats_time_ = stats_.start_time;
    }

    MulticastReceiver::MulticastReceiver(const ReceiverConfig& config)
        : MulticastReceiver()
    {
        initialize(config);
    }

    MulticastReceiver::~MulticastReceiver()
    {
        stop();
    }

    bool MulticastReceiver::initialize(const ReceiverConfig& config)
    {
        if (running_.load())
        {
            return false;
        }

        config_ = config;
        if (config_.enable_logging && !config_.log_file_path.empty())
        {
            log_file_ = std::make_unique<std::ofstream>(config_.log_file_path, std::ios::app);
            if (!log_file_->is_open())
            {
                std::cerr << "Failed to open log file: " << config_.log_file_path << std::endl;
                return false;
            }
        }

        socket_ = std::make_unique<MulticastSocket>();
        return socket_->create_and_join(config_.multicast_ip, config_.multicast_port,
                                        config_.interface_ip, config_.receive_buffer_size);
    }

    bool MulticastReceiver::start()
    {
        if (running_.load() || !socket_ || !socket_->is_valid())
        {
            return false;
        }

        running_.store(true);
        receiver_thread_ = std::thread(&MulticastReceiver::receiver_loop, this);
        stats_thread_ = std::thread([this]()
        {
            while (running_.load())
            {
                std::this_thread::sleep_for(config_.stats_interval);
                if (running_.load())
                {
                    print_stats();
                }
            }
        });

        return true;
    }

    void MulticastReceiver::stop()
    {
        if (!running_.load())
        {
            return;
        }

        running_.store(false);

        if (receiver_thread_.joinable())
        {
            receiver_thread_.join();
        }

        if (stats_thread_.joinable())
        {
            stats_thread_.join();
        }

        if (socket_)
        {
            socket_->close();
        }

        if (log_file_)
        {
            log_file_->close();
        }
    }

    void MulticastReceiver::receiver_loop()
    {
        constexpr size_t BUFFER_SIZE = 65536;
        const auto buffer = std::make_unique<char[]>(BUFFER_SIZE);

        log_message("MulticastReceiver started - listening on " +
            config_.multicast_ip + ":" + std::to_string(config_.multicast_port));

        while (running_.load())
        {
            ssize_t bytes_received = socket_->receive_data(buffer.get(), BUFFER_SIZE);

            if (bytes_received > 0)
            {
                process_message(buffer.get(), static_cast<size_t>(bytes_received));
                stats_.total_messages_received++;
                stats_.total_bytes_received += bytes_received;
            }
            else if (bytes_received < 0)
            {
                if (running_.load())
                {
                    log_message("Error receiving data from multicast socket");
                }
                break;
            }
        }

        log_message("MulticastReceiver stopped");
    }

    void MulticastReceiver::process_message(const void* data, const size_t length)
    {
        if (length < sizeof(MessageHeader))
        {
            stats_.invalid_messages++;
            log_message("Received invalid message: too small (" + std::to_string(length) + " bytes)");
            return;
        }

        const auto* header = static_cast<const MessageHeader*>(data);
        if (header->message_length != length)
        {
            stats_.invalid_messages++;
            log_message("Message length mismatch: header says " +
                std::to_string(header->message_length) +
                ", received " + std::to_string(length));
            return;
        }
        if (config_.validate_sequence_numbers && last_sequence_number_ > 0)
        {
            if (header->sequence_number != last_sequence_number_ + 1)
            {
                uint64_t gap = header->sequence_number - last_sequence_number_ - 1;
                stats_.sequence_gaps += gap;
                log_message("Sequence gap detected: expected " +
                    std::to_string(last_sequence_number_ + 1) +
                    ", got " + std::to_string(header->sequence_number) +
                    " (gap: " + std::to_string(gap) + ")");
            }
        }
        last_sequence_number_ = header->sequence_number;
        std::string debug_str = format_message_debug(*header, data);
        log_message(debug_str);
        if (message_handler_)
        {
            message_handler_(*header, data, length);
        }
    }

    std::string MulticastReceiver::format_message_debug(const MessageHeader& header, const void* data)
    {
        std::ostringstream oss;
        oss << "[" << message_utils::format_timestamp(header.timestamp_ns) << "] ";

        switch (static_cast<MessageType>(header.message_type))
        {
        case MessageType::HEARTBEAT:
            {
                const auto* msg = reinterpret_cast<const HeartbeatMessage*>(data);
                oss << msg->toDebugString();
                break;
            }
        case MessageType::PRICE_LEVEL_UPDATE:
            {
                const auto* msg = reinterpret_cast<const PriceLevelUpdateMessage*>(data);
                oss << msg->toDebugString();
                break;
            }
        case MessageType::PRICE_LEVEL_DELETE:
            {
                const auto* msg = reinterpret_cast<const PriceLevelDeleteMessage*>(data);
                oss << msg->toDebugString();
                break;
            }
        case MessageType::TRADE:
            {
                const auto* msg = reinterpret_cast<const TradeMessage*>(data);
                oss << msg->toDebugString();
                break;
            }
        case MessageType::SNAPSHOT_BEGIN:
            {
                const auto* msg = reinterpret_cast<const SnapshotBeginMessage*>(data);
                oss << msg->toDebugString();
                break;
            }
        case MessageType::SNAPSHOT_ENTRY:
            {
                const auto* msg = reinterpret_cast<const SnapshotEntryMessage*>(data);
                oss << msg->toDebugString();
                break;
            }
        case MessageType::SNAPSHOT_END:
            {
                const auto* msg = reinterpret_cast<const SnapshotEndMessage*>(data);
                oss << msg->toDebugString();
                break;
            }
        case MessageType::BOOK_CLEAR:
            {
                const auto* msg = reinterpret_cast<const BookClearMessage*>(data);
                oss << msg->toDebugString();
                break;
            }
        default:
            {
                oss << "UNKNOWN_MESSAGE: " << header.toDebugString();
                break;
            }
        }

        return oss.str();
    }

    void MulticastReceiver::log_message(const std::string& message) const
    {
        if (!config_.enable_logging)
        {
            return;
        }

        if (config_.log_to_console)
        {
            std::cout << message << std::endl;
        }

        if (log_file_ && log_file_->is_open())
        {
            *log_file_ << message << std::endl;
            log_file_->flush();
        }
    }

    void MulticastReceiver::print_stats()
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - stats_.start_time);

        std::ostringstream oss;
        oss << "\n=== RECEIVER STATISTICS ===\n"
            << "Runtime: " << elapsed.count() << "s\n"
            << "Total Messages: " << stats_.total_messages_received << "\n"
            << "Total Bytes: " << stats_.total_bytes_received << "\n"
            << "Sequence Gaps: " << stats_.sequence_gaps << "\n"
            << "Invalid Messages: " << stats_.invalid_messages << "\n";

        if (elapsed.count() > 0)
        {
            oss << "Msg/sec: " << (stats_.total_messages_received / elapsed.count()) << "\n"
                << "MB/sec: " << std::fixed << std::setprecision(2)
                << (stats_.total_bytes_received / (1024.0 * 1024.0) / elapsed.count()) << "\n";
        }
        oss << "==========================\n";
        log_message(oss.str());
        last_stats_time_ = now;
    }

    void MulticastReceiver::reset_stats()
    {
        stats_ = Stats{};
        stats_.start_time = std::chrono::steady_clock::now();
        last_stats_time_ = stats_.start_time;
        last_sequence_number_ = 0;
    }
}
