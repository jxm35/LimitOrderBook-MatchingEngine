#include "spdlog/spdlog.h"

#include "OrderBook.h"

OrderBook::OrderBook(const Security &instrument) : instrument_(instrument) {
    askLimits_ = std::map<long, std::shared_ptr<Limit>, std::less<>>();
    bidLimits_ = std::map<long, std::shared_ptr<Limit>, std::greater<>>();
    orders_ = std::unordered_map<long, std::shared_ptr<OrderBookEntry>>();
    ordersMatched_ = 0;
}

size_t OrderBook::Count() {
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
    return {bestBid, bestAsk};
}

boost::optional<std::shared_ptr<Limit>> OrderBook::GetBestBidLimit() {
    if (bidLimits_.empty()) {
        return boost::none;
    }
    return bidLimits_.begin()->second;
}

boost::optional<std::shared_ptr<Limit>> OrderBook::GetBestAskLimit() {
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

void OrderBook::AddOrder(const Order &order) {
    order.IsBuy() ?
    AddOrder(order, order.Price(), bidLimits_, orders_)
                  : AddOrder(order, order.Price(), askLimits_, orders_);
}

void OrderBook::ChangeOrder(ModifyOrder modifyOrder) {
    if (orders_.contains(modifyOrder.OrderId())) {
        auto obe = orders_.at(modifyOrder.OrderId());
        RemoveOrder(modifyOrder.ToCancelOrder());
        obe->CurrentOrder().IsBuy() ?
        AddOrder(modifyOrder.ToNewOrder(), obe->CurrentOrder().Price(), bidLimits_, orders_)
                                    : AddOrder(modifyOrder.ToNewOrder(), obe->CurrentOrder().Price(), askLimits_,
                                               orders_);
    }

}

void OrderBook::RemoveOrder(const CancelOrder &cancelOrder) {
    if (orders_.contains(cancelOrder.OrderId())) {
        auto obe = orders_.at(cancelOrder.OrderId());
        obe->CurrentOrder().IsBuy() ? RemoveOrder(cancelOrder.OrderId(), obe, bidLimits_, orders_)
                                    : RemoveOrder(cancelOrder.OrderId(), obe, askLimits_, orders_);

    } else {
        throw std::invalid_argument("order id not found");
    }
}

std::list<OrderBookEntry> OrderBook::GetAskOrders() {
    std::list < OrderBookEntry > orderBookEntries;
    for (const auto &askLimit: askLimits_) {
        if (askLimit.second->IsEmpty()) {
            continue;
        }
        std::shared_ptr<OrderBookEntry> askLimitPtr = askLimit.second->head_;
        while (askLimitPtr != nullptr) {
            orderBookEntries.push_back(*askLimitPtr);
            askLimitPtr = askLimitPtr->next;
        }
    }
    return orderBookEntries;
}

std::list<OrderBookEntry> OrderBook::GetBidOrders() {
    std::list < OrderBookEntry > orderBookEntries;
    for (const auto &bidLimit: bidLimits_) {
        if (bidLimit.second->IsEmpty()) {
            continue;
        }
        std::shared_ptr<OrderBookEntry> bidLimitPtr = bidLimit.second->head_;
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
    std::list < OrderStruct > orders;
    for (const auto &limit: askLimits_) {
        orders.splice(orders.end(), limit.second->GetOrderRecords());
    }
    for (const auto &limit: bidLimits_) {
        orders.splice(orders.end(), limit.second->GetOrderRecords());
    }
    return orders;
}

template<typename sort>
void OrderBook::AddOrder(Order order, long price,
                         std::map<long, std::shared_ptr<Limit>, sort> &limitLevels,
                         std::unordered_map<long, std::shared_ptr<OrderBookEntry>> &internalOrderBook) {

    auto orderId = order.OrderId();
    auto lim = limitLevels.find(price);
    if (lim != limitLevels.end()) {
        auto orderBookEntry = std::make_shared<OrderBookEntry>(lim->second, order);
        lim->second->AddOrder(orderBookEntry);
        internalOrderBook[orderId] = orderBookEntry;
    } else {
        auto limit = std::make_shared<Limit>(price);
        auto orderBookEntry = std::make_shared<OrderBookEntry>(limit, order);
        limit->AddOrder(orderBookEntry);
        limitLevels[price] = limit;
        internalOrderBook[orderId] = orderBookEntry;
    }
}

template<typename sort>
void OrderBook::RemoveOrder(long orderId, const std::shared_ptr<OrderBookEntry> &obe,
                            std::map<long, std::shared_ptr<Limit>, sort> &limitLevels,
                            std::unordered_map<long, std::shared_ptr<OrderBookEntry>> &internalOrderBook) {

    auto limit = obe->GetLimit();
    if (!limit) {
        internalOrderBook.erase(orderId);
        return;
    }

    if (limit->GetOrderCount() == 1) {
        limitLevels.erase(obe->CurrentOrder().Price());
        internalOrderBook.erase(orderId);
        return;
    }

    auto prev = obe->previous.lock();
    auto next = obe->next;

    if (prev && next) {
        next->previous = obe->previous;
        prev->next = next;
    } else if (prev) {
        prev->next = nullptr;
    } else if (next) {
        next->previous.reset();
    }

    limit->RemoveOrder(obe->CurrentOrder().OrderId(), obe->CurrentOrder().CurrentQuantity());
    internalOrderBook.erase(orderId);
}

void OrderBook::PlaceMarketBuyOrder(uint32_t quantity) {
    std::list<std::shared_ptr<OrderBookEntry>> toRemove;
    if (askLimits_.empty()) {
        return;
    }
    for (std::pair<long, std::shared_ptr<Limit>> limit: askLimits_) {
        auto askPtr = limit.second->head_;
        while (askPtr != nullptr) {
            if (quantity == askPtr->CurrentOrder().CurrentQuantity()) {
                spdlog::info("market buy order filled @ {} pence", limit.first);
                spdlog::info("sell order {} filled @ {} pence", askPtr->CurrentOrder().OrderId(),
                             limit.first);
                toRemove.push_back(askPtr);
                ordersMatched_ += quantity;
                quantity = 0;
                break;
            } else if (quantity < askPtr->CurrentOrder().CurrentQuantity()) {
                askPtr->DecreaseQuantity(quantity);
                spdlog::info("market buy order filled @ {} pence", limit.first);
                spdlog::info("sell order {} partially filled @ {} pence", askPtr->CurrentOrder().OrderId(),
                             limit.first);
                ordersMatched_ += quantity;
                quantity = 0;
                break;
            } else {
                quantity -= askPtr->CurrentOrder().CurrentQuantity();
                spdlog::info("market buy order partially filled @ {} pence", limit.first);
                spdlog::info("sell order {} filled @ {} pence", askPtr->CurrentOrder().OrderId(),
                             limit.first);
                toRemove.push_back(askPtr);
                ordersMatched_ += askPtr->CurrentOrder().CurrentQuantity();
                askPtr = askPtr->next;
            }

        }
        if (askLimits_.empty() || quantity == 0) {
            break;
        }
    }
    for (const auto &obe: toRemove) {
        RemoveOrder(obe->CurrentOrder().OrderId(), obe, askLimits_, orders_);
    }
}

void OrderBook::PlaceMarketSellOrder(uint32_t quantity) {
    std::list<std::shared_ptr<OrderBookEntry>> toRemove;
    if (bidLimits_.empty()) {
        return;
    }
    for (std::pair<long, std::shared_ptr<Limit>> limit: bidLimits_) {
        auto bidPtr = limit.second->head_;
        while (bidPtr != nullptr) {
            if (quantity == bidPtr->CurrentOrder().CurrentQuantity()) {
                spdlog::info("market sell order filled @ {} pence", limit.first);
                spdlog::info("buy order {} filled @ {} pence", bidPtr->CurrentOrder().OrderId(),
                             limit.first);
                toRemove.push_back(bidPtr);
                ordersMatched_ += quantity;
                quantity = 0;
                break;
            } else if (quantity < bidPtr->CurrentOrder().CurrentQuantity()) {
                bidPtr->DecreaseQuantity(quantity);
                spdlog::info("market sell order filled @ {} pence", limit.first);
                spdlog::info("buy order {} partially filled @ {} pence", bidPtr->CurrentOrder().OrderId(),
                             limit.first);
                ordersMatched_ += quantity;
                quantity = 0;
                break;
            } else {
                quantity -= bidPtr->CurrentOrder().CurrentQuantity();
                spdlog::info("market sell order partially filled @ {} pence", limit.first);
                spdlog::info("buy order {} filled @ {} pence", bidPtr->CurrentOrder().OrderId(),
                             limit.first);
                toRemove.push_back(bidPtr);
                ordersMatched_ += bidPtr->CurrentOrder().CurrentQuantity();
                bidPtr = bidPtr->next;
            }

        }
        if (bidLimits_.empty() || quantity == 0) {
            break;
        }
    }
    for (const auto &obe: toRemove) {
        RemoveOrder(obe->CurrentOrder().OrderId(), obe, bidLimits_, orders_);
    }
}

MatchResult OrderBook::Match() {
    if (bidLimits_.empty() || askLimits_.empty()) {
        return {};
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
                ordersMatched_ += askPtr->CurrentOrder().CurrentQuantity();
                auto next = askPtr->next;
                RemoveOrder(askPtr->CurrentOrder().OrderId(), askPtr, askLimits_, orders_);
                askPtr = next;
            } else if (bidPtr->CurrentOrder().CurrentQuantity() < askPtr->CurrentOrder().CurrentQuantity()) {
                askPtr->DecreaseQuantity(bidPtr->CurrentOrder().CurrentQuantity());
                bestAsk->DecreaseQuantity(bidPtr->CurrentOrder().CurrentQuantity());
                spdlog::info("sell order {} partially filled @ {} pence", askPtr->CurrentOrder().OrderId(),
                             bidPrice);
                spdlog::info("buy order {} filled @ {} pence", bidPtr->CurrentOrder().OrderId(),
                             bidPrice);
                ordersMatched_ += bidPtr->CurrentOrder().CurrentQuantity();
                auto next = bidPtr->next;
                RemoveOrder(bidPtr->CurrentOrder().OrderId(), bidPtr, bidLimits_, orders_);
                bidPtr = next;
            } else {
                spdlog::info("sell order {} filled @ {} pence", askPtr->CurrentOrder().OrderId(),
                             bidPrice);
                spdlog::info("buy order {} filled @ {} pence", bidPtr->CurrentOrder().OrderId(),
                             bidPrice);
                // remove bidPtr + askPtr;
                ordersMatched_ += askPtr->CurrentOrder().CurrentQuantity();
                auto next = askPtr->next;
                RemoveOrder(askPtr->CurrentOrder().OrderId(), askPtr, askLimits_, orders_);
                askPtr = next;
                next = bidPtr->next;
                RemoveOrder(bidPtr->CurrentOrder().OrderId(), bidPtr, bidLimits_, orders_);
                bidPtr = next;
            }
        }
    }
    return {};
}
