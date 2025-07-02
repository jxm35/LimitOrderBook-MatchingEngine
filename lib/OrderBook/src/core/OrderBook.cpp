#include "spdlog/spdlog.h"

#include "core/OrderBook.h"

#include "publisher/MarketDataPublisher.h"

namespace mdfeed {
    class NullMarketDataPublisher;
}

template<typename MarketDataPublisher>
OrderBook<MarketDataPublisher>::OrderBook(const Security &instrument,
                                          mdfeed::MDAdapter<MarketDataPublisher> md_adapter) : instrument_(
        instrument), md_adapter_(md_adapter) {
    askLimits_ = std::map<long, std::shared_ptr<Limit>, std::less<>>();
    bidLimits_ = std::map<long, std::shared_ptr<Limit>, std::greater<>>();
    orders_ = std::unordered_map<long, std::shared_ptr<OrderBookEntry>>();
    matchedQuantity_ = 0;
}

template<typename MarketDataPublisher>
size_t OrderBook<MarketDataPublisher>::Count() {
    return orders_.size();
}

template<typename MarketDataPublisher>
bool OrderBook<MarketDataPublisher>::ContainsOrder(long orderId) {
    return orders_.contains(orderId);
}

template<typename MarketDataPublisher>
OrderBookSpread OrderBook<MarketDataPublisher>::GetSpread() {
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

template<typename MarketDataPublisher>
boost::optional<std::shared_ptr<Limit>> OrderBook<MarketDataPublisher>::GetBestBidLimit() {
    if (bidLimits_.empty()) {
        return boost::none;
    }
    return bidLimits_.begin()->second;
}

template<typename MarketDataPublisher>
boost::optional<std::shared_ptr<Limit>> OrderBook<MarketDataPublisher>::GetBestAskLimit() {
    if (askLimits_.empty()) {
        return boost::none;
    }
    return askLimits_.begin()->second;
}

template<typename MarketDataPublisher>
boost::optional<long> OrderBook<MarketDataPublisher>::GetBestBidPrice() {
    if (bidLimits_.empty()) {
        return boost::none;
    }
    return bidLimits_.begin()->first;
}

template<typename MarketDataPublisher>
boost::optional<long> OrderBook<MarketDataPublisher>::GetBestAskPrice() {
    if (askLimits_.empty()) {
        return boost::none;
    }
    return askLimits_.begin()->first;
}

template<typename MarketDataPublisher>
void OrderBook<MarketDataPublisher>::AddOrder(const Order &order) {
    order.IsBuy()
    ? AddOrder(order, order.Price(), bidLimits_, orders_)
    : AddOrder(order, order.Price(), askLimits_, orders_);
}

template<typename MarketDataPublisher>
void OrderBook<MarketDataPublisher>::AmendOrder(const long orderId, const Order &order) {
    RemoveOrder(orderId);
    order.IsBuy()
    ? AddOrder(order, order.Price(), bidLimits_, orders_)
    : AddOrder(order, order.Price(), askLimits_, orders_);
}

template<typename MarketDataPublisher>
void OrderBook<MarketDataPublisher>::RemoveOrder(const long orderId) {
    if (orders_.contains(orderId)) {
        auto obe = orders_.at(orderId);
        const bool isBuy = obe->CurrentOrder().IsBuy();
        const long price = obe->CurrentOrder().Price();
        if (RemoveOrder(orderId, obe)) {
            if (isBuy) {
                bidLimits_.erase(price);
            } else {
                askLimits_.erase(price);
            }
        }
    } else {
        throw std::invalid_argument("order id not found");
    }
}

template<typename MarketDataPublisher>
std::list<OrderBookEntry> OrderBook<MarketDataPublisher>::GetAskOrders() {
    std::list<OrderBookEntry> orderBookEntries;
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

template<typename MarketDataPublisher>
std::list<OrderBookEntry> OrderBook<MarketDataPublisher>::GetBidOrders() {
    std::list<OrderBookEntry> orderBookEntries;
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

template<typename MarketDataPublisher>
std::map<long, uint32_t> OrderBook<MarketDataPublisher>::GetBidQuantities() {
    std::map<long, uint32_t> limitQuantities;
    for (const auto &bidLimit: bidLimits_) {
        if (bidLimit.second->IsEmpty()) {
            continue;
        }
        limitQuantities[bidLimit.first] = bidLimit.second->GetOrderQuantity();
    }
    return limitQuantities;
}

template<typename MarketDataPublisher>
std::map<long, uint32_t> OrderBook<MarketDataPublisher>::GetAskQuantities() {
    std::map<long, uint32_t> limitQuantities;
    for (const auto &Limit: askLimits_) {
        if (Limit.second->IsEmpty()) {
            continue;
        }
        limitQuantities[Limit.first] = Limit.second->GetOrderQuantity();
    }
    return limitQuantities;
}

template<typename MarketDataPublisher>
std::list<OrderStruct> OrderBook<MarketDataPublisher>::GetOrders() {
    std::list<OrderStruct> orders;
    for (const auto &limit: askLimits_) {
        orders.splice(orders.end(), limit.second->GetOrderRecords());
    }
    for (const auto &limit: bidLimits_) {
        orders.splice(orders.end(), limit.second->GetOrderRecords());
    }
    return orders;
}

template<typename MarketDataPublisher>
template<typename Sort>
void OrderBook<MarketDataPublisher>::AddOrder(Order order, long price,
                                              std::map<long, std::shared_ptr<Limit>, Sort> &limitLevels,
                                              std::unordered_map<long, std::shared_ptr<OrderBookEntry>> &internalOrderBook) {
    if (order.IsBuy()) {
        this->TryMatch(order, price, askLimits_);
    } else {
        this->TryMatch(order, price, bidLimits_);
    }
    if (order.CurrentQuantity() == 0) {
        return;
    }
    auto it = limitLevels.find(price);
    std::shared_ptr<Limit> limit;
    if (it != limitLevels.end()) {
        limit = it->second;
    } else {
        limit = std::make_shared<Limit>(price);
        limitLevels[price] = limit;
    }
    auto entry = std::make_shared<OrderBookEntry>(limit, order);
    limit->AddOrder(entry);
    internalOrderBook[order.OrderId()] = entry;
}

template<typename MarketDataPublisher>
bool OrderBook<MarketDataPublisher>::RemoveOrder(long orderId, const std::shared_ptr<OrderBookEntry> &obe) {
    auto limit = obe->GetLimit();
    if (!limit) {
        orders_.erase(orderId);
        return false;
    }

    if (limit->GetOrderCount() == 1) {
        orders_.erase(orderId);
        return true;
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
    orders_.erase(orderId);
    return false;
}

template<typename MarketDataPublisher>
void OrderBook<MarketDataPublisher>::PlaceMarketBuyOrder(uint32_t quantity) {
    if (askLimits_.empty()) {
        spdlog::info("No asks available to fill market buy order");
        return;
    }

    Order marketOrder{{"username", 1}, std::numeric_limits<long>::max(), quantity, true};
    const uint32_t remaining = TryMatch(marketOrder, marketOrder.Price(), askLimits_);
    if (remaining > 0) {
        spdlog::info("Market buy order partially filled, {} units remaining unfilled", remaining);
    } else {
        spdlog::info("Market buy order completely filled");
    }
}

template<typename MarketDataPublisher>
void OrderBook<MarketDataPublisher>::PlaceMarketSellOrder(uint32_t quantity) {
    if (bidLimits_.empty()) {
        spdlog::info("No bids available to fill market sell order");
        return;
    }

    Order marketOrder{{"username", 1}, std::numeric_limits<long>::min(), quantity, false};
    const uint32_t remaining = TryMatch(marketOrder, marketOrder.Price(), bidLimits_);
    if (remaining > 0) {
        spdlog::info("Market sell order partially filled, {} units remaining unfilled", remaining);
    } else {
        spdlog::info("Market sell order completely filled");
    }
}

template<typename MarketDataPublisher>
template<typename LimitMap>
uint32_t OrderBook<MarketDataPublisher>::TryMatch(Order &incomingOrder, long price, LimitMap &opposingLimits) {
    const bool isBuy = incomingOrder.IsBuy();
    auto opposingIter = opposingLimits.begin();
    uint32_t remainingQty = incomingOrder.CurrentQuantity();
    bool erasedLimit = false;
    while (opposingIter != opposingLimits.end() && remainingQty > 0) {
        long opposingPrice = opposingIter->first;

        if ((isBuy && price < opposingPrice) ||
            (!isBuy && price > opposingPrice)) { // TODO: replace with side sign comparison
            break;
        }

        auto limit = opposingIter->second;
        auto opposingOrderPtr = limit->head_;
        while (opposingOrderPtr && remainingQty > 0) {
            auto &restingOrder = opposingOrderPtr->CurrentOrder();
            const uint32_t restingQty = restingOrder.CurrentQuantity();
            const uint32_t matchedQty = std::min(restingQty, remainingQty);

            opposingOrderPtr->DecreaseQuantity(matchedQty);
            limit->DecreaseQuantity(matchedQty);
            md_adapter_.notify_price_level_change(opposingPrice, restingQty - matchedQty, restingQty,
                                                  !isBuy); // TODO: This should happen within the limit

            spdlog::debug("{} order {} {}filled @ {} pence", isBuy ? "buy" : "sell", incomingOrder.OrderId(),
                          matchedQty < remainingQty ? "partially " : "", opposingPrice);
            spdlog::debug("{} order {} {}filled @ {} pence", isBuy ? "sell" : "buy", restingOrder.OrderId(),
                          matchedQty < restingQty ? "partially " : "", opposingPrice);

            matchedQuantity_ += matchedQty;
            remainingQty -= matchedQty;
            incomingOrder.DecreaseQuantity(matchedQty);

            if (restingOrder.CurrentQuantity() == 0) {
                auto next = opposingOrderPtr->next;
                if (RemoveOrder(restingOrder.OrderId(), opposingOrderPtr)) {
                    opposingIter = opposingLimits.erase(opposingIter);
                    erasedLimit = true;
                }
                opposingOrderPtr = next;
                continue;
            } else {
                break;
            }
        }
        if (!erasedLimit) {
            ++opposingIter;
        } else {
            erasedLimit = false;
        }
    }
    return remainingQty;
}

template
class OrderBook<mdfeed::NullMarketDataPublisher>;

template
class OrderBook<mdfeed::MarketDataPublisher>;
