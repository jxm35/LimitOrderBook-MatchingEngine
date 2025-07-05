#pragma once

#include "messages/OrderMessages.h"
#include <string>

namespace orderentry {
    class OrderEntryClient {
    public:
        OrderEntryClient();
        ~OrderEntryClient();

        bool connect(const std::string& server_address, uint16_t port);
        void disconnect();
        [[nodiscard]] bool is_connected() const { return socket_fd_ >= 0; }

        bool send_new_order(uint64_t client_order_id, const std::string& symbol,
                            Side side, uint64_t price, uint64_t quantity);

        bool send_cancel_order(uint64_t client_order_id,
                               uint64_t original_order_id,
                               const std::string& symbol);

    private:
        bool send_message(const void* data, size_t length);

        int socket_fd_;
        uint64_t sequence_number_;
    };
} // namespace orderentry
