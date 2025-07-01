
#include "entries/OrderBookEntry.h"

OrderBookEntry::OrderBookEntry(std::shared_ptr<Limit> parentLimit, Order currentOrder)
        : currentOrder_(currentOrder) {
    limit_ = parentLimit;
}

Limit::Limit(long price) {
    price_ = price;
    size_ = 0;
    orderQuantity_ = 0;
    head_ = nullptr;
    tail_ = nullptr;
}

void Limit::AddOrder(std::shared_ptr<OrderBookEntry> order) {
    if (head_ == nullptr) {
        // no orders on this level
        head_ = order;
        tail_ = order;
    } else {
        // we have orders on this level
        std::shared_ptr<OrderBookEntry> tailEntry = tail_;
        tailEntry->next = order;
        order->previous = tailEntry;
        tail_ = order;
    }
    size_++;
    orderQuantity_ += order->CurrentOrder().CurrentQuantity();
}

std::expected<void, std::string> Limit::RemoveOrder(long orderId, uint32_t quantity) {
    if (!head_) [[unlikely]] {
        return std::unexpected("Order not found - limit is empty");
    }
    auto current = head_;
    while (current && current->CurrentOrder().OrderId() != orderId) {
        current = current->next;
    }
    if (!current) {
        return std::unexpected("Order not found");
    }
    if (current == head_) {
        head_ = head_->next;
        if (head_) {
            head_->previous.reset();
        }
    }
    if (current == tail_) {
        tail_ = tail_->previous.lock();
        if (tail_) {
            tail_->next = nullptr;
        }
    }
    if (auto prev = current->previous.lock()) {
        prev->next = current->next;
    }
    if (current->next) {
        current->next->previous = current->previous;
    }
    size_--;
    orderQuantity_ -= quantity;
    return {};
}

std::list<OrderStruct> Limit::GetOrderRecords() const {
    std::list<OrderStruct> orderRecords;
    auto entryPtr = head_;
    uint32_t queuePosition = 0;
    while (entryPtr) {
        Order currentOrder = entryPtr->CurrentOrder();
        if (currentOrder.CurrentQuantity() != 0) {
            orderRecords.push_back(OrderStruct{
                    currentOrder.OrderId(),
                    currentOrder.CurrentQuantity(),
                    currentOrder.Price(),
                    currentOrder.IsBuy(),
                    currentOrder.Username(),
                    currentOrder.SecurityID(),
                    queuePosition,
            });
            queuePosition++;
            entryPtr = entryPtr->next;
        }
    }
    return orderRecords;
}
