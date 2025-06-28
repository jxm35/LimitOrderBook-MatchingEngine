#pragma once

#include <chrono>

namespace mdfeed {

#pragma pack(push, 1)

struct MessageHeader {
    uint64_t sequence_number;
    uint32_t message_length;
    uint16_t message_type;
    uint64_t timestamp_ns;
    uint32_t instrument_id;
};

enum class MessageType : uint16_t {
    HEARTBEAT = 1,
    PRICE_LEVEL_UPDATE = 2,
    PRICE_LEVEL_DELETE = 3,
    TRADE = 4,
    SNAPSHOT_BEGIN = 5,
    SNAPSHOT_ENTRY = 6,
    SNAPSHOT_END = 7,
    BOOK_CLEAR = 8
};

enum class Side : uint8_t {
    BUY = 1,
    SELL = 2
};

enum class UpdateAction : uint8_t {
    NEW = 1,
    CHANGE = 2,
    DELETE = 3
};

struct HeartbeatMessage {
    MessageHeader header;
};

struct PriceLevelUpdateMessage {
    MessageHeader header;
    uint64_t price;
    uint64_t quantity;
    Side side;
    UpdateAction action;
    uint8_t reserved[6];
};

struct PriceLevelDeleteMessage {
    MessageHeader header;
    uint64_t price;
    Side side;
    uint8_t reserved[7];
};

struct TradeMessage {
    MessageHeader header;
    uint64_t trade_id;
    uint64_t price;
    uint64_t quantity;
    Side aggressor_side;
    uint8_t reserved[7];
};

struct SnapshotBeginMessage {
    MessageHeader header;
    uint32_t total_entries;
    uint8_t reserved[4];
};

struct SnapshotEntryMessage {
    MessageHeader header;
    uint64_t price;
    uint64_t quantity;
    Side side;
    uint8_t reserved[7];
};

struct SnapshotEndMessage {
    MessageHeader header;
    uint32_t checksum;
    uint8_t reserved[4];
};

struct BookClearMessage {
    MessageHeader header;
    uint32_t reason_code;
    uint8_t reserved[4];
};

#pragma pack(pop)

namespace message_utils {

inline uint64_t get_timestamp_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
}

template<typename T>
 void init_header(T& message, MessageType type, uint64_t seq_num, uint32_t instrument_id) {
    message.header.sequence_number = seq_num;
    message.header.message_length = sizeof(T);
    message.header.message_type = static_cast<uint16_t>(type);
    message.header.timestamp_ns = get_timestamp_ns();
    message.header.instrument_id = instrument_id;
}

}

}