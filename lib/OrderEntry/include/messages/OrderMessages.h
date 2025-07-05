#pragma once

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

namespace orderentry {
#pragma pack(push, 1)

    struct MessageHeader {
        uint64_t sequence_number;
        uint32_t message_length;
        uint16_t message_type;
        uint64_t timestamp_ns;
        uint32_t instrument_id;

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "Header[seq=" << sequence_number
                << ", len=" << message_length << ", type=" << message_type
                << ", ts=" << timestamp_ns << ", inst=" << instrument_id << "]";
            return oss.str();
        }
    };

    enum class MessageType : uint16_t {
        NEW_ORDER = 1,
        CANCEL_ORDER = 2,
        MODIFY_ORDER = 3,
        ORDER_ACK = 4,
        ORDER_REJECT = 5,
        EXECUTION_REPORT = 6
    };

    enum class Side : uint8_t { BUY = 1, SELL = 2 };

    enum class OrderType : uint8_t { MARKET = 1, LIMIT = 2 };

    enum class TimeInForce : uint8_t { DAY = 1, IOC = 2 };

    struct NewOrderMessage {
        MessageHeader header;
        uint64_t client_order_id;
        uint64_t price;
        uint64_t quantity;
        Side side;
        OrderType order_type;
        TimeInForce time_in_force;
        char symbol[8];
        uint8_t reserved[5];

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "NEW_ORDER: " << header.toDebugString()
                << " client_order_id=" << client_order_id
                << ", price=" << std::fixed << std::setprecision(2)
                << (price / 100.0) << ", qty=" << quantity
                << ", side=" << (side == Side::BUY ? "BUY" : "SELL")
                << ", type="
                << (order_type == OrderType::MARKET ? "MARKET" : "LIMIT")
                << ", symbol=" << std::string(symbol, 8);
            return oss.str();
        }
    };

    struct CancelOrderMessage {
        MessageHeader header;
        uint64_t client_order_id;
        uint64_t original_client_order_id;
        char symbol[8];
        uint8_t reserved[8];

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "CANCEL_ORDER: " << header.toDebugString()
                << " client_order_id=" << client_order_id
                << ", orig_id=" << original_client_order_id
                << ", symbol=" << std::string(symbol, 8);
            return oss.str();
        }
    };

    struct ModifyOrderMessage {
        MessageHeader header;
        uint64_t client_order_id;
        uint64_t original_client_order_id;
        uint64_t new_price;
        uint64_t new_quantity;
        char symbol[8];
        uint8_t reserved[8];

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "MODIFY_ORDER: " << header.toDebugString()
                << " client_order_id=" << client_order_id
                << ", orig_id=" << original_client_order_id
                << ", new_price=" << std::fixed << std::setprecision(2)
                << (new_price / 100.0) << ", new_qty=" << new_quantity
                << ", symbol=" << std::string(symbol, 8);
            return oss.str();
        }
    };

    struct OrderAckMessage {
        MessageHeader header;
        uint64_t client_order_id;
        uint64_t exchange_order_id;
        char symbol[8];
        uint8_t reserved[8];

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "ORDER_ACK: " << header.toDebugString()
                << " client_order_id=" << client_order_id
                << ", exchange_order_id=" << exchange_order_id
                << ", symbol=" << std::string(symbol, 8);
            return oss.str();
        }
    };

    struct OrderRejectMessage {
        MessageHeader header;
        uint64_t client_order_id;
        uint32_t reject_reason;
        char reject_text[16];
        char symbol[8];
        uint8_t reserved[4];

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "ORDER_REJECT: " << header.toDebugString()
                << " client_order_id=" << client_order_id
                << ", reason=" << reject_reason
                << ", text=" << std::string(reject_text, 16)
                << ", symbol=" << std::string(symbol, 8);
            return oss.str();
        }
    };

    struct ExecutionReportMessage {
        MessageHeader header;
        uint64_t client_order_id;
        uint64_t exchange_order_id;
        uint64_t execution_id;
        uint64_t execution_price;
        uint64_t execution_quantity;
        uint64_t leaves_quantity;
        Side side;
        uint8_t reserved[7];

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "EXECUTION_REPORT: " << header.toDebugString()
                << " client_order_id=" << client_order_id
                << ", exchange_order_id=" << exchange_order_id
                << ", exec_id=" << execution_id << ", exec_price=" << std::fixed
                << std::setprecision(2) << (execution_price / 100.0)
                << ", exec_qty=" << execution_quantity
                << ", leaves_qty=" << leaves_quantity
                << ", side=" << (side == Side::BUY ? "BUY" : "SELL");
            return oss.str();
        }
    };

#pragma pack(pop)

    namespace message_utils {
        inline uint64_t get_timestamp_ns()
        {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                           std::chrono::high_resolution_clock::now()
                                   .time_since_epoch())
                    .count();
        }

        template<typename T> void init_header(T& message, MessageType type,
                                              uint64_t seq_num,
                                              uint32_t instrument_id)
        {
            message.header.sequence_number = seq_num;
            message.header.message_length = sizeof(T);
            message.header.message_type = static_cast<uint16_t>(type);
            message.header.timestamp_ns = get_timestamp_ns();
            message.header.instrument_id = instrument_id;
        }

        inline std::string format_timestamp(uint64_t timestamp_ns)
        {
            const uint64_t timestamp_ms = timestamp_ns / 1000000;
            const uint64_t ms = timestamp_ms % 1000;
            const std::time_t time_seconds = timestamp_ms / 1000;

            std::tm* tm_ptr = std::localtime(&time_seconds);

            std::ostringstream oss;
            oss << std::put_time(tm_ptr, "%H:%M:%S") << '.' << std::setw(3)
                << std::setfill('0') << ms;
            return oss.str();
        }

        inline std::string message_type_to_string(MessageType type)
        {
            switch (type) {
            case MessageType::NEW_ORDER:
                return "NEW_ORDER";
            case MessageType::CANCEL_ORDER:
                return "CANCEL_ORDER";
            case MessageType::MODIFY_ORDER:
                return "MODIFY_ORDER";
            case MessageType::ORDER_ACK:
                return "ORDER_ACK";
            case MessageType::ORDER_REJECT:
                return "ORDER_REJECT";
            case MessageType::EXECUTION_REPORT:
                return "EXECUTION_REPORT";
            default:
                return "UNKNOWN";
            }
        }
    } // namespace message_utils
} // namespace orderentry
