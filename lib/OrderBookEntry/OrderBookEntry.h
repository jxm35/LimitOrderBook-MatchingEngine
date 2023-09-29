#pragma once

#include <chrono>
#include <ctime>
#include <list>

#include "order.h"

class Limit;

class OrderBookEntry;

class OrderBookEntry {
private:
    Order currentOrder_;
    Limit *limit_;
    std::time_t creationTime_;
public:
    OrderBookEntry *next;
    OrderBookEntry *previous;

    OrderBookEntry(class Limit *parentLimit, Order currentOrder);

    OrderBookEntry() {};  // ToFix

    void DecreaseQuantity(uint16_t quantity) {
        currentOrder_.DecreaseQuantity(quantity);
    }

    Order CurrentOrder() const {
        return currentOrder_;
    }

    Limit *Limit() const {
        return limit_;
    }

    std::time_t CreationTime() {
        return creationTime_;
    }

//    bool operator==(const OrderBookEntry &rhs) {
//        return this->currentOrder_.OrderId() == rhs.currentOrder_.OrderId();
//    }
//
//    bool operator!=(const OrderBookEntry &rhs) {
//        return this->currentOrder_.OrderId() != rhs.currentOrder_.OrderId();
//    }

};


enum Side {
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
    uint32_t QueuePosition;
};

class Limit {
private:
    long price_;
    long size_;
    // this has been stored as a field so we don't have to traverse a linked list every time we want to check order quantity at a level
    uint32_t orderQuantity_;

public:
    Limit(long price);

    // mutable to allow us to change these in a logically const Limit (in a set)
    OrderBookEntry mutable *head_;
    OrderBookEntry mutable *tail_;

    bool IsEmpty() const {
        return head_ == nullptr && tail_ == nullptr;
    }

    inline long Price() const {
        return price_;
    }

    void AddOrder(OrderBookEntry *obe);

    void RemoveOrder(long orderId, uint32_t quantity);

    inline void DecreaseQuantity(uint32_t quantity) {
        if (quantity > orderQuantity_)
            throw std::invalid_argument("removing too much");
        orderQuantity_ -= quantity;
    }

    Side Side() {
        if (IsEmpty())
            return Side::Unknown;
        if (head_->CurrentOrder().IsBuy()) {
            return Side::Bid;
        } else {
            return Side::Ask;
        }
    }


    uint32_t GetOrderCount() const {
//        uint16_t orderCount = 0;
//        OrderBookEntry *entryPtr = head_;
//        while (entryPtr != nullptr) {
//            if (entryPtr->CurrentOrder().CurrentQuantity() != 0) {
//                orderCount++;
//                entryPtr = entryPtr->next;
//            }
//        }
//        return orderCount;
        return size_;
    }

    uint32_t
    GetOrderQuantity() const {   // JamesFix consider storing this as a field on the limit, so as not to have to repeatedly traverse this list.
//        uint16_t orderQuantity = 0;
//        OrderBookEntry *entryPtr = head_;
//        while (entryPtr != nullptr) {
//            orderQuantity += entryPtr->CurrentOrder().CurrentQuantity();
//            entryPtr = entryPtr->next;
//        }
//        return orderQuantity;
        return orderQuantity_;
    }

    std::list<OrderStruct> GetOrderRecords() {
        std::list<OrderStruct> orderRecords;
        OrderBookEntry *entryPtr = head_;
        uint32_t queuePosition = 0;
        while (entryPtr != nullptr) {
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

};

//         orderRecords.sort([](const OrderStruct &f, const OrderStruct &s) { return f.price < s.price; })

//struct SortBids {
//    bool operator () (const OrderStruct &f, const OrderStruct &s) const {
//        return f.price > s.price;
//    }
//};
struct SortAsks {
    bool operator()(const Limit &f, const Limit &s) const {
        return f.Price() < s.Price();
//        return f.price < s.price;
    }
};

struct SortBids {
    bool operator()(const Limit &f, const Limit &s) const {
        return f.Price() > s.Price();
    }
};
//struct SortBids {
//    bool operator () (const Limit *f, const Limit *s) const {
//        return f->Price() > s->Price();
//    }
//};
//





