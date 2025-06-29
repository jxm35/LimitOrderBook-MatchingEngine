#pragma once

#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>

namespace mdfeed
{
#pragma pack(push, 1)

    struct MessageHeader
    {
        uint64_t sequence_number;
        uint32_t message_length;
        uint16_t message_type;
        uint64_t timestamp_ns;
        uint32_t instrument_id;

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "Header[seq=" << sequence_number
                << ", len=" << message_length
                << ", type=" << message_type
                << ", ts=" << timestamp_ns
                << ", inst=" << instrument_id << "]";
            return oss.str();
        }
    };

    enum class MessageType : uint16_t
    {
        HEARTBEAT = 1,
        PRICE_LEVEL_UPDATE = 2,
        PRICE_LEVEL_DELETE = 3,
        TRADE = 4,
        SNAPSHOT_BEGIN = 5,
        SNAPSHOT_ENTRY = 6,
        SNAPSHOT_END = 7,
        BOOK_CLEAR = 8
    };

    enum class Side : uint8_t
    {
        BUY = 1,
        SELL = 2
    };

    enum class UpdateAction : uint8_t
    {
        NEW = 1,
        CHANGE = 2,
        DELETE = 3
    };

    struct HeartbeatMessage
    {
        MessageHeader header;

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "HEARTBEAT: " << header.toDebugString();
            return oss.str();
        }
    };

    struct PriceLevelUpdateMessage
    {
        MessageHeader header;
        uint64_t price;
        uint64_t quantity;
        Side side;
        UpdateAction action;
        uint8_t reserved[6];

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "PRICE_UPDATE: " << header.toDebugString()
                << " price=" << std::fixed << std::setprecision(8) << (price / 100000000.0)
                << ", qty=" << quantity
                << ", side=" << (side == Side::BUY ? "BUY" : "SELL")
                << ", action=" << (action == UpdateAction::NEW
                                       ? "NEW"
                                       : action == UpdateAction::CHANGE
                                       ? "CHANGE"
                                       : "DELETE");
            return oss.str();
        }
    };

    struct PriceLevelDeleteMessage
    {
        MessageHeader header;
        uint64_t price;
        Side side;
        uint8_t reserved[7];

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "PRICE_DELETE: " << header.toDebugString()
                << " price=" << std::fixed << std::setprecision(8) << (price / 100000000.0)
                << ", side=" << (side == Side::BUY ? "BUY" : "SELL");
            return oss.str();
        }
    };

    struct TradeMessage
    {
        MessageHeader header;
        uint64_t trade_id;
        uint64_t price;
        uint64_t quantity;
        Side aggressor_side;
        uint8_t reserved[7];

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "TRADE: " << header.toDebugString()
                << " trade_id=" << trade_id
                << ", price=" << std::fixed << std::setprecision(8) << (price / 100000000.0)
                << ", qty=" << quantity
                << ", aggressor=" << (aggressor_side == Side::BUY ? "BUY" : "SELL");
            return oss.str();
        }
    };

    struct SnapshotBeginMessage
    {
        MessageHeader header;
        uint32_t total_entries;
        uint8_t reserved[4];

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "SNAPSHOT_BEGIN: " << header.toDebugString()
                << " total_entries=" << total_entries;
            return oss.str();
        }
    };

    struct SnapshotEntryMessage
    {
        MessageHeader header;
        uint64_t price;
        uint64_t quantity;
        Side side;
        uint8_t reserved[7];

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "SNAPSHOT_ENTRY: " << header.toDebugString()
                << " price=" << std::fixed << std::setprecision(8) << (price / 100000000.0)
                << ", qty=" << quantity
                << ", side=" << (side == Side::BUY ? "BUY" : "SELL");
            return oss.str();
        }
    };

    struct SnapshotEndMessage
    {
        MessageHeader header;
        uint32_t checksum;
        uint8_t reserved[4];

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "SNAPSHOT_END: " << header.toDebugString()
                << " checksum=" << std::hex << checksum << std::dec;
            return oss.str();
        }
    };

    struct BookClearMessage
    {
        MessageHeader header;
        uint32_t reason_code;
        uint8_t reserved[4];

        [[nodiscard]] std::string toDebugString() const
        {
            std::ostringstream oss;
            oss << "BOOK_CLEAR: " << header.toDebugString()
                << " reason_code=" << reason_code;
            return oss.str();
        }
    };

#pragma pack(pop)

    namespace message_utils
    {
        inline uint64_t get_timestamp_ns()
        {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()
            ).count();
        }

        template <typename T>
        void init_header(T& message, MessageType type, uint64_t seq_num, uint32_t instrument_id)
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
            oss << std::put_time(tm_ptr, "%H:%M:%S")
                << '.' << std::setw(3) << std::setfill('0') << ms;

            return oss.str();
        }

        inline std::string message_type_to_string(MessageType type)
        {
            switch (type)
            {
            case MessageType::HEARTBEAT: return "HEARTBEAT";
            case MessageType::PRICE_LEVEL_UPDATE: return "PRICE_LEVEL_UPDATE";
            case MessageType::PRICE_LEVEL_DELETE: return "PRICE_LEVEL_DELETE";
            case MessageType::TRADE: return "TRADE";
            case MessageType::SNAPSHOT_BEGIN: return "SNAPSHOT_BEGIN";
            case MessageType::SNAPSHOT_ENTRY: return "SNAPSHOT_ENTRY";
            case MessageType::SNAPSHOT_END: return "SNAPSHOT_END";
            case MessageType::BOOK_CLEAR: return "BOOK_CLEAR";
            default: return "UNKNOWN";
            }
        }
    }
}
