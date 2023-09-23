
#include "OrderBook.h"

OrderBook::OrderBook(Security instrument) {
    instrument_ = instrument;
    askLimits_ = std::set<Limit, SortAsks>();
    bidLimits_ = std::set<Limit, SortBids>();
    orders_ = std::unordered_map<long, OrderBookEntry >();
}

int OrderBook::Count() {
    return orders_.size();
}

bool OrderBook::ContainsOrder(long orderId) {
    return orders_.contains(orderId);
}

OrderBookSpread OrderBook::GetSpread() {
    boost::optional<long> bestAsk = boost::none;
    boost::optional<long> bestBid = boost::none;
    if (!askLimits_.empty() && askLimits_.begin()->head_ != nullptr) {
        bestAsk = askLimits_.begin()->Price();
    }
    if (!bidLimits_.empty() && bidLimits_.begin()->head_ != nullptr) {
        bestBid = bidLimits_.begin()->Price();
    }
    return OrderBookSpread(bestBid, bestAsk);
}

void OrderBook::AddOrder(Order order) {
    Limit baseLimit(order.Price());
    if (order.IsBuy()) {
        AddOrderInner(order, baseLimit, bidLimits_, orders_);
    } else {
        AddOrderInner(order, baseLimit, askLimits_, orders_);
    }
}

void OrderBook::ChangeOrder(ModifyOrder modifyOrder) {
    if (orders_.contains(modifyOrder.orderId_)) {
        OrderBookEntry obe = orders_.at(modifyOrder.orderId_);
        RemoveOrder(modifyOrder.ToCancelOrder());
        if (obe.CurrentOrder().IsBuy()) {
            AddOrderInner(modifyOrder.ToNewOrder(),  *obe.Limit(), bidLimits_, orders_);
        } else {
            AddOrderInner(modifyOrder.ToNewOrder(), *obe.Limit(), askLimits_, orders_);
        }
    }

}

void OrderBook::RemoveOrder(CancelOrder cancelOrder) {
    if (orders_.contains(cancelOrder.orderId_)) {
        OrderBookEntry obe = orders_.at(cancelOrder.orderId_);
        RemoveOrderInner(cancelOrder.orderId_, &obe, orders_);

    }

}

std::list<OrderBookEntry*> OrderBook::GetAskOrders() {
    std::list<OrderBookEntry*> orderBookEntries;
    for(const auto & askLimit : askLimits_) {
        if (askLimit.IsEmpty()) {
            continue;
        }
        OrderBookEntry *askLimitPtr = askLimit.head_;
        orderBookEntries.push_back(askLimitPtr);
        while (askLimitPtr != nullptr) {
            orderBookEntries.push_back(askLimitPtr);
            askLimitPtr = askLimitPtr->next;
        }
    }
    return orderBookEntries;
}

std::list<OrderBookEntry> OrderBook::GetBidOrders() {
    std::list<OrderBookEntry> orderBookEntries;
    for(const auto & bidLimit : bidLimits_) {
        if (bidLimit.IsEmpty()) {
            continue;
        }
        OrderBookEntry *bidLimitPtr = bidLimit.head_;
        while (bidLimitPtr != nullptr) {
            OrderBookEntry obe = *bidLimitPtr;
            orderBookEntries.push_back(obe);
            bidLimitPtr = bidLimitPtr->next;
        }
    }
    return orderBookEntries;
}

void OrderBook::AddOrderInner(Order order,Limit baseLimit, std::set<Limit, SortAsks>& limitLevels,
                              std::unordered_map<long, OrderBookEntry>& internalOrderBook) {
    if (limitLevels.contains(baseLimit)) {
        auto lim = limitLevels.find(baseLimit);
        if (lim != limitLevels.end()) {
            OrderBookEntry orderBookEntry(&baseLimit, order);
            if (lim->head_ == nullptr) {
                // no orders on this level
                lim->head_ = &orderBookEntry;
                lim->tail_ = &orderBookEntry;
            } else {
                // we have orders on this level
                OrderBookEntry *tailEntry = lim->tail_;
                tailEntry->next = &orderBookEntry;
                orderBookEntry.previous = tailEntry;
                lim->tail_ = &orderBookEntry;
            }
        } else {
            throw "we are supposed to have a limit";
        }
    } else {
        // level does not exist
        limitLevels.insert(baseLimit);
        OrderBookEntry orderBookEntry(&baseLimit, order);
        baseLimit.head_ = &orderBookEntry;
        baseLimit.tail_ = &orderBookEntry;
        internalOrderBook[order.orderId_] = orderBookEntry;
    }
}
void OrderBook::AddOrderInner(Order order,Limit baseLimit, std::set<Limit, SortBids>& limitLevels,
                              std::unordered_map<long, OrderBookEntry>& internalOrderBook) {
    if (limitLevels.contains(baseLimit)) {
        auto lim = limitLevels.find(baseLimit);
        if (lim != limitLevels.end()) {
            OrderBookEntry *orderBookEntry = new OrderBookEntry(&baseLimit, order);
            if (lim->head_ == nullptr) {
                // no orders on this level
                lim->head_ = orderBookEntry;
                lim->tail_ = orderBookEntry;
            } else {
                // we have orders on this level
                OrderBookEntry *tailEntry = lim->tail_;
                tailEntry->next = orderBookEntry;
                orderBookEntry->previous = tailEntry;
                lim->tail_ = orderBookEntry;
            }
        } else {
            throw "we are supposed to have a limit";
        }
    } else {
        // level does not exist
        OrderBookEntry *orderBookEntry = new OrderBookEntry(&baseLimit, order);
        baseLimit.head_ = orderBookEntry;
        baseLimit.tail_ = orderBookEntry;
        limitLevels.insert(baseLimit);
        internalOrderBook[order.orderId_] = *orderBookEntry;
    }
}

void OrderBook::RemoveOrderInner(long orderId, OrderBookEntry *obe,
                                 std::unordered_map<long, OrderBookEntry> internalOrderBook) {
    // remove from limit linked list
    if (obe->previous != nullptr &&obe->next != nullptr) {
        obe->next->previous = obe->previous;
        obe->previous->next = obe->next;
    } else if (obe->previous != nullptr) {
        obe->previous->next = nullptr;
    } else if (obe->next != nullptr){
        obe->next->previous = nullptr;
    }

    // remove from limit obj
    if (obe->Limit()->head_ == obe && obe->Limit()->tail_ == obe) {
        obe->Limit()->head_ = nullptr;
        obe->Limit()->tail_ = nullptr;
    } else if (obe->Limit()->head_ == obe) {
        obe->Limit()->head_ = obe->next;
    } else if (obe->Limit()->tail_ == obe) {
        obe->Limit()->tail_ = obe->previous;
    }
    internalOrderBook.erase(orderId);

}
