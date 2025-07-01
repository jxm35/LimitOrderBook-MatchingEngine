#pragma once

#include <chrono>
#include <memory>
#include <list>
#include <optional>
#include <expected>

#include "orders/Order.h"

class OrderBookEntry;

enum class Side {
    Unknown,
    Bid,
    Ask
};

struct OrderStruct {
    long orderId;
    uint32_t quantity;
    long price;
    bool isBuySide;
    std::string username;
    int securityId;
    uint32_t queuePosition;
};

class Limit {
private:
    long price_;
    long size_;
    uint32_t orderQuantity_;

public:
    explicit Limit(long price);

    std::shared_ptr<OrderBookEntry> head_;
    std::shared_ptr<OrderBookEntry> tail_;

    [[nodiscard]] bool IsEmpty() const noexcept {
        return !head_ && !tail_;
    }

    [[nodiscard]] long Price() const noexcept {
        return price_;
    }

    void AddOrder(std::shared_ptr<OrderBookEntry> orderBookEntry);

    std::expected<void, std::string> RemoveOrder(long orderId, uint32_t quantity);

    void DecreaseQuantity(uint32_t quantity) {
        if (quantity > orderQuantity_) [[unlikely]] {
            throw std::invalid_argument("removing too much");
        }
        orderQuantity_ -= quantity;
    }

    [[nodiscard]] uint32_t GetOrderCount() const noexcept {
        return size_;
    }

    [[nodiscard]] uint32_t GetOrderQuantity() const noexcept {
        return orderQuantity_;
    }

    [[nodiscard]] std::list<OrderStruct> GetOrderRecords() const;
};

class OrderBookEntry {
private:
    Order currentOrder_;
    std::weak_ptr<Limit> limit_;
    std::chrono::time_point<std::chrono::steady_clock> creationTime_;

public:
    std::shared_ptr<OrderBookEntry> next;
    std::weak_ptr<OrderBookEntry> previous;

    OrderBookEntry(std::shared_ptr<Limit> parentLimit, Order currentOrder);

    OrderBookEntry() = delete;

    void DecreaseQuantity(uint16_t quantity) {
        currentOrder_.DecreaseQuantity(quantity);
    }

    [[nodiscard]] const Order &CurrentOrder() const noexcept {
        return currentOrder_;
    }

    [[nodiscard]] std::shared_ptr<Limit> GetLimit() const {
        return limit_.lock();
    }
};
