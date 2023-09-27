#include "spdlog/spdlog.h"

#include "OrderBook.h"

OrderBook::OrderBook(Security instrument) {
    instrument_ = instrument;
    askLimits_ = std::map<long, Limit *, std::less<>>();
    bidLimits_ = std::map<long, Limit *, std::greater<>>();
    orders_ = std::unordered_map<long, OrderBookEntry>();
    ordersMatched_ = 0;
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
std::map<long, uint32_t> OrderBook::GetBidQuantities() {
    std::map<long, uint32_t> limitQuantities;
    for (const auto &bidLimit: bidLimits_) {
        if (bidLimit.second->IsEmpty()) {
            continue;
        }
        limitQuantities[bidLimit.first] = bidLimit.second->GetOrderQuantity();
    }
    return limitQuantities;
}
std::map<long, uint32_t> OrderBook::GetAskQuantities() {
    std::map<long, uint32_t> limitQuantities;
    for (const auto &Limit: askLimits_) {
        if (Limit.second->IsEmpty()) {
            continue;
        }
        limitQuantities[Limit.first] = Limit.second->GetOrderQuantity();
    }
    return limitQuantities;
}

std::list<OrderStruct> OrderBook::GetOrders() {
    std::list<OrderStruct> orders;
    for (const auto &limit: askLimits_) {
        orders.splice(orders.end(), limit.second->GetOrderRecords());
    }
    for (const auto &limit: bidLimits_) {
        orders.splice(orders.end(), limit.second->GetOrderRecords());
    }
}

template<typename sort>
void OrderBook::AddOrder(Order order, long price, std::map<long, Limit *, sort> &limitLevels,
                         std::unordered_map<long, OrderBookEntry> &internalOrderBook) {
    if (limitLevels.contains(price)) {
        auto lim = limitLevels.find(price);
        if (lim != limitLevels.end()) {
            OrderBookEntry *orderBookEntry = new OrderBookEntry(lim->second, order);
            lim->second->AddOrder(orderBookEntry);
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

void OrderBook::PlaceMarketBuyOrder(uint32_t quantity) {
    std::list<OrderBookEntry *> toRemove;
    if (askLimits_.empty()) {
        return;
    }
        for(std::pair<long, Limit*> limit: askLimits_) {
            auto askPtr = limit.second->head_;
            while (askPtr != nullptr) {
                if (quantity == askPtr->CurrentOrder().CurrentQuantity()) {
                    spdlog::info("market buy order filled @ {} pence", limit.first);
                    spdlog::info("sell order {} filled @ {} pence", askPtr->CurrentOrder().OrderId(),
                                 limit.first);
                    toRemove.push_back(askPtr);
                    ordersMatched_++;
                    quantity=0;
                    break;
                } else if (quantity < askPtr->CurrentOrder().CurrentQuantity()) {
                    askPtr->DecreaseQuantity(quantity);
                    spdlog::info("market buy order filled @ {} pence", limit.first);
                    spdlog::info("sell order {} partially filled @ {} pence", askPtr->CurrentOrder().OrderId(),
                                 limit.first);
                    ordersMatched_++;
                    quantity=0;
                    break;
                } else {
                    quantity -= askPtr->CurrentOrder().CurrentQuantity();
                    spdlog::info("market buy order partially filled @ {} pence", limit.first);
                    spdlog::info("sell order {} filled @ {} pence", askPtr->CurrentOrder().OrderId(),
                                 limit.first);
                    toRemove.push_back(askPtr);
                    ordersMatched_++;
                    askPtr = askPtr->next;
                }

            }
            if (askLimits_.empty() || quantity == 0) {
                break;
            }
        }
    for(auto obe: toRemove) {
        RemoveOrder(obe->CurrentOrder().OrderId(), *obe, askLimits_, orders_);
    }
}

void OrderBook::PlaceMarketSellOrder(uint32_t quantity) {
    std::list<OrderBookEntry *> toRemove;
    if (bidLimits_.empty()) {
        return;
    }
    for(std::pair<long, Limit*> limit: bidLimits_) {
        auto bidPtr = limit.second->head_;
        while (bidPtr != nullptr) {
            if (quantity == bidPtr->CurrentOrder().CurrentQuantity()) {
                spdlog::info("market sell order filled @ {} pence", limit.first);
                spdlog::info("buy order {} filled @ {} pence", bidPtr->CurrentOrder().OrderId(),
                             limit.first);
                toRemove.push_back(bidPtr);
                ordersMatched_++;
                quantity=0;
                break;
            } else if (quantity < bidPtr->CurrentOrder().CurrentQuantity()) {
                bidPtr->DecreaseQuantity(quantity);
                spdlog::info("market sell order filled @ {} pence", limit.first);
                spdlog::info("buy order {} partially filled @ {} pence", bidPtr->CurrentOrder().OrderId(),
                             limit.first);
                ordersMatched_++;
                quantity = 0;
                break;
            } else {
                quantity -= bidPtr->CurrentOrder().CurrentQuantity();
                spdlog::info("market sell order partially filled @ {} pence", limit.first);
                spdlog::info("buy order {} filled @ {} pence", bidPtr->CurrentOrder().OrderId(),
                             limit.first);
                toRemove.push_back(bidPtr);
                ordersMatched_++;
                bidPtr = bidPtr->next;
            }

        }
        if (bidLimits_.empty() || quantity == 0) {
            break;
        }
    }
    for(auto obe: toRemove) {
        RemoveOrder(obe->CurrentOrder().OrderId(), *obe, bidLimits_, orders_);
    }
}

MatchResult OrderBook::Match() {
    if (bidLimits_.empty() || askLimits_.empty()) {
        return MatchResult();
    }
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
                ordersMatched_++;
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
                ordersMatched_++;
            } else {
                spdlog::info("sell order {} filled @ {} pence", askPtr->CurrentOrder().OrderId(),
                             bidPrice);
                spdlog::info("buy order {} filled @ {} pence", bidPtr->CurrentOrder().OrderId(),
                             bidPrice);
                // remove bidPtr + askPtr
                auto next = askPtr->next;
                auto askQuantity = askPtr->CurrentOrder().CurrentQuantity();
                RemoveOrder(askPtr->CurrentOrder().OrderId(), *askPtr, askLimits_, orders_);
                askPtr = next;
                next = bidPtr->next;
                RemoveOrder(bidPtr->CurrentOrder().OrderId(), *bidPtr, bidLimits_, orders_);
                bidPtr = next;
                ordersMatched_+=2;
            }
        }
    }
    return MatchResult();
}
