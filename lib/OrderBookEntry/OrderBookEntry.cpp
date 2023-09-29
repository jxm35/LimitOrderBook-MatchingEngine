
#include "OrderBookEntry.h"

OrderBookEntry::OrderBookEntry(class Limit *parentLimit, Order currentOrder)
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

void Limit::AddOrder(OrderBookEntry *order) {
    if (head_ == nullptr) {
        // no orders on this level
        head_ = order;
        tail_ = order;
    } else {
        // we have orders on this level
        OrderBookEntry *tailEntry = tail_;
        tailEntry->next = order;
        order->previous = tailEntry;
        tail_ = order;
    }
    size_++;
    orderQuantity_ += order->CurrentOrder().CurrentQuantity();
}

void Limit::RemoveOrder(long orderId, uint32_t quantity) {
    if (head_->CurrentOrder().OrderId() == orderId) {
        head_ = head_->next;
    }
    if (tail_->CurrentOrder().OrderId() == orderId) {
        tail_ = tail_->previous;
    }
    size_--;
    orderQuantity_ -= quantity;
}