#include "spdlog/spdlog.h"

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
        bestAsk = askLimits_.begin()->first;
    }
    if (!bidLimits_.empty() && bidLimits_.begin()->second->head_ != nullptr) {
        bestBid = bidLimits_.begin()->first;
    }
    return OrderBookSpread(bestBid, bestAsk);
}

boost::optional<Limit *> OrderBook::GetBestBidLimit() {
    if (bidLimits_.empty()) {
        return boost::none;
    }
    return bidLimits_.begin()->second;
}

boost::optional<Limit *> OrderBook::GetBestAskLimit() {
    if (askLimits_.empty()) {
        return boost::none;
    }
    return askLimits_.begin()->second;
}

boost::optional<long> OrderBook::GetBestBidPrice() {
    if (bidLimits_.empty()) {
        return boost::none;
    }
    return bidLimits_.begin()->first;
}
boost::optional<long> OrderBook::GetBestAskPrice() {
    if (askLimits_.empty()) {
        return boost::none;
    }
    return askLimits_.begin()->first;
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
        obe.CurrentOrder().IsBuy() ? RemoveOrder(cancelOrder.OrderId(), obe, bidLimits_, orders_)
                                   : RemoveOrder(cancelOrder.OrderId(), obe, askLimits_, orders_);

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
            lim->second->AddOrder(orderBookEntry);
//            if (lim->second->head_ == nullptr) {
//                // no orders on this level
//                lim->second->head_ = orderBookEntry;
//                lim->second->tail_ = orderBookEntry;
//            } else {
//                // we have orders on this level
//                OrderBookEntry *tailEntry = lim->second->tail_;
//                tailEntry->next = orderBookEntry;
//                orderBookEntry->previous = tailEntry;
//                lim->second->tail_ = orderBookEntry;
//            }
            internalOrderBook[order.OrderId()] = *orderBookEntry;
        } else {
            throw std::invalid_argument("we are supposed to have a limit");
        }
    } else {
        // level does not exist
        Limit *limit = new Limit(price);
        OrderBookEntry *orderBookEntry = new OrderBookEntry(limit, order);
        limit->AddOrder(orderBookEntry);
        limitLevels[price] = limit;
        internalOrderBook[order.OrderId()] = *orderBookEntry;
    }
}

template<typename sort>
void OrderBook::RemoveOrder(long orderId, const OrderBookEntry &obe, std::map<long, Limit *, sort> &limitLevels,
                            std::unordered_map<long, OrderBookEntry> &internalOrderBook) {
    if (obe.Limit()->GetOrderCount() == 1) {
        limitLevels.erase(obe.CurrentOrder().Price());
        delete (obe.Limit()->head_);
        delete (obe.Limit());
        internalOrderBook.erase(orderId);
        return;
    }
    // remove from limit linked list
    if (obe.previous != nullptr && obe.next != nullptr) {
        obe.next->previous = obe.previous;
        obe.previous->next = obe.next;
    } else if (obe.previous != nullptr) {
        obe.previous->next = nullptr;
    } else if (obe.next != nullptr) {
        obe.next->previous = nullptr;
    }

    // remove from limit obj
    obe.Limit()->RemoveOrder(obe.CurrentOrder().OrderId(), obe.CurrentOrder().CurrentQuantity());
    internalOrderBook.erase(orderId);
}

MatchResult OrderBook::Match() {
    auto bestBid = bidLimits_.begin()->second;
    auto bestAsk = askLimits_.begin()->second;
    long bidPrice = bestBid->Price();
    if (bidPrice >= bestAsk->Price()) {
        auto bidPtr = bestBid->head_;
        auto askPtr = bestAsk->head_;
        while (bidPtr != nullptr && askPtr != nullptr) {
            if (bidPtr->CurrentOrder().CurrentQuantity() > askPtr->CurrentOrder().CurrentQuantity()) {
                bidPtr->DecreaseQuantity(askPtr->CurrentOrder().CurrentQuantity());
                bestBid->DecreaseQuantity(askPtr->CurrentOrder().CurrentQuantity());
                spdlog::info("buy order {} partially filled @ {} pence", bidPtr->CurrentOrder().OrderId(),
                             bidPrice);
                spdlog::info("sell order {}  filled @ {} pence", askPtr->CurrentOrder().OrderId(),
                             bidPrice);
                auto next = askPtr->next;
                RemoveOrder(askPtr->CurrentOrder().OrderId(), *askPtr, askLimits_, orders_);
                askPtr = next;
            } else if (bidPtr->CurrentOrder().CurrentQuantity() < askPtr->CurrentOrder().CurrentQuantity()) {
                askPtr->DecreaseQuantity(bidPtr->CurrentOrder().CurrentQuantity());
                bestAsk->DecreaseQuantity(bidPtr->CurrentOrder().CurrentQuantity());
                spdlog::info("sell order {} partially filled @ {} pence", askPtr->CurrentOrder().OrderId(),
                             bidPrice);
                spdlog::info("buy order {} filled @ {} pence", bidPtr->CurrentOrder().OrderId(),
                             bidPrice);
                auto next = bidPtr->next;
                RemoveOrder(bidPtr->CurrentOrder().OrderId(), *bidPtr, bidLimits_, orders_);
                bidPtr = next;
            } else {
                spdlog::info("sell order {} filled @ {} pence", askPtr->CurrentOrder().OrderId(),
                             bidPrice);
                spdlog::info("buy order {} filled @ {} pence", bidPtr->CurrentOrder().OrderId(),
                             bidPrice);
                // remove bidPtr + askPtr
                auto next = askPtr->next;
                auto askQuantity = askPtr->CurrentOrder().CurrentQuantity();
                RemoveOrder(askPtr->CurrentOrder().OrderId(), *askPtr, askLimits_, orders_);
//                bestAsk->DecreaseQuantity(bidPtr->CurrentOrder().CurrentQuantity());
                askPtr = next;
                next = bidPtr->next;
                RemoveOrder(bidPtr->CurrentOrder().OrderId(), *bidPtr, bidLimits_, orders_);
//                bestBid->DecreaseQuantity(askQuantity);
                bidPtr = next;
            }
        }
    }
    return MatchResult();
}
