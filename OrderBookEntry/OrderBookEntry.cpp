
#include "OrderBookEntry.h"

OrderBookEntry::OrderBookEntry(class Limit *parentLimit, Order currentOrder)
        : currentOrder_(currentOrder) {
    limit_ = parentLimit;
}

Limit::Limit(long price) {
    price_ = price;
    head_ = nullptr;
    tail_ = nullptr;
}
