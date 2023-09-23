
#include "OrderBook.h"

OrderBook::OrderBook(Security instrument) {
    instrument_ = instrument;
    askLimits_ = std::map<long, Limit *, std::less<>>();
    bidLimits_ = std::map<long, Limit *, std::greater<>>();
    orders_ = std::unordered_map<long, OrderBookEntry>();
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
    if (!askLimits_.empty() && askLimits_.begin()->second->head_ != nullptr) {
        bestAsk = askLimits_.begin()->second->Price();
    }
    if (!bidLimits_.empty() && bidLimits_.begin()->second->head_ != nullptr) {
        bestBid = bidLimits_.begin()->second->Price();
    }
    return OrderBookSpread(bestBid, bestAsk);
}

void OrderBook::AddOrder(Order order) {
    order.IsBuy() ?
    AddOrder(order, order.Price(), bidLimits_, orders_)
                  : AddOrder(order, order.Price(), askLimits_, orders_);
}

void OrderBook::ChangeOrder(ModifyOrder modifyOrder) {
    if (orders_.contains(modifyOrder.OrderId())) {
        OrderBookEntry obe = orders_.at(modifyOrder.OrderId());
        RemoveOrder(modifyOrder.ToCancelOrder());
        obe.CurrentOrder().IsBuy() ?
        AddOrder(modifyOrder.ToNewOrder(), obe.CurrentOrder().Price(), bidLimits_, orders_)
                                   : AddOrder(modifyOrder.ToNewOrder(), obe.CurrentOrder().Price(), askLimits_,
                                              orders_);
    }

}

void OrderBook::RemoveOrder(CancelOrder cancelOrder) {
    if (orders_.contains(cancelOrder.OrderId())) {
        OrderBookEntry obe = orders_.at(cancelOrder.OrderId());
        RemoveOrderInner(cancelOrder.OrderId(), &obe, orders_);
    } else {
        throw std::invalid_argument("order id not found");
    }
}

std::list<OrderBookEntry> OrderBook::GetAskOrders() {
    std::list<OrderBookEntry> orderBookEntries;
    for (const auto &askLimit: askLimits_) {
        if (askLimit.second->IsEmpty()) {
            continue;
        }
        OrderBookEntry *askLimitPtr = askLimit.second->head_;
        while (askLimitPtr != nullptr) {
            orderBookEntries.push_back(*askLimitPtr);
            askLimitPtr = askLimitPtr->next;
        }
    }
    return orderBookEntries;
}

std::list<OrderBookEntry> OrderBook::GetBidOrders() {
    std::list<OrderBookEntry> orderBookEntries;
    for (const auto &bidLimit: bidLimits_) {
        if (bidLimit.second->IsEmpty()) {
            continue;
        }
        OrderBookEntry *bidLimitPtr = bidLimit.second->head_;
        while (bidLimitPtr != nullptr) {
            orderBookEntries.push_back(*bidLimitPtr);
            bidLimitPtr = bidLimitPtr->next;
        }
    }
    return orderBookEntries;
}

template<typename sort>
void OrderBook::AddOrder(Order order, long price, std::map<long, Limit *, sort> &limitLevels,
                         std::unordered_map<long, OrderBookEntry> &internalOrderBook) {
    if (limitLevels.contains(price)) {
        auto lim = limitLevels.find(price);
        if (lim != limitLevels.end()) {
            OrderBookEntry *orderBookEntry = new OrderBookEntry(lim->second, order);
            if (lim->second->head_ == nullptr) {
                // no orders on this level
                lim->second->head_ = orderBookEntry;
                lim->second->tail_ = orderBookEntry;
            } else {
                // we have orders on this level
                OrderBookEntry *tailEntry = lim->second->tail_;
                tailEntry->next = orderBookEntry;
                orderBookEntry->previous = tailEntry;
                lim->second->tail_ = orderBookEntry;
            }
            internalOrderBook[order.OrderId()] = *orderBookEntry;
        } else {
            throw "we are supposed to have a limit";
        }
    } else {
        // level does not exist
        Limit *limit = new Limit(price);
        OrderBookEntry *orderBookEntry = new OrderBookEntry(limit, order);
        limit->head_ = orderBookEntry;
        limit->tail_ = orderBookEntry;
        limitLevels[price] = limit;
        internalOrderBook[order.OrderId()] = *orderBookEntry;
    }
}

void OrderBook::RemoveOrderInner(long orderId, OrderBookEntry *obe,
                                 std::unordered_map<long, OrderBookEntry> &internalOrderBook) {
    // remove from limit linked list
    if (obe->previous != nullptr && obe->next != nullptr) {
        obe->next->previous = obe->previous;
        obe->previous->next = obe->next;
    } else if (obe->previous != nullptr) {
        obe->previous->next = nullptr;
    } else if (obe->next != nullptr) {
        obe->next->previous = nullptr;
    }

    // remove from limit obj
    if (obe->Limit()->head_->CurrentOrder().OrderId() == obe->CurrentOrder().OrderId() &&
        obe->Limit()->tail_->CurrentOrder().OrderId() == obe->CurrentOrder().OrderId()) {
        obe->Limit()->head_ = nullptr;
        obe->Limit()->tail_ = nullptr;
    } else if (obe->Limit()->head_->CurrentOrder().OrderId() == obe->CurrentOrder().OrderId()) {
        obe->Limit()->head_ = obe->next;
    } else if (obe->Limit()->tail_->CurrentOrder().OrderId() == obe->CurrentOrder().OrderId()) {
        obe->Limit()->tail_ = obe->previous;
    }
    internalOrderBook.erase(orderId);
}
