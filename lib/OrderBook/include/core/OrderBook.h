#pragma once

#include <boost/optional.hpp>
#include <list>
#include <unordered_map>
#include <map>

#include "orders/Order.h"
#include "entries/OrderBookEntry.h"
#include "securities/Security.h"
#include "publisher/MDAdapter.h"

class OrderBookSpread {
private:
    boost::optional<long> bid_;
    boost::optional<long> ask_;

public:
    OrderBookSpread(boost::optional<long> bid, boost::optional<long> ask) {
        bid_ = bid;
        ask_ = ask;
    }

    boost::optional<long> Spread() {
        if (bid_.has_value() && ask_.has_value()) {
            return ask_.value() - bid_.value();
        }
        return boost::none;
    }
};

template<typename MarketDataPublisher>
class OrderBook {
private:
    Security instrument_;
    long matchedQuantity_;
    mdfeed::MDAdapter<MarketDataPublisher> md_adapter_;

    // sorted maps
    // limits could also be implemented as an array with pointers to the best bid and ask limit.
    // or a buy and a sell limit array for fast lookup, as most used limits will be near the centre (price wise) (so near the edge of a tree)
    std::map<long, std::shared_ptr<Limit>, std::less<>> askLimits_;
    std::map<long, std::shared_ptr<Limit>, std::greater<>> bidLimits_;

    // dictionary
    // could switch this for an array, with order_id as the index (as we can re start order ids for each day of trading).
    // This would also allow us to pre-allocate the storage fif we have enough memory to further improve performance. (std::vector)
    std::unordered_map<long, std::shared_ptr<OrderBookEntry>> orders_;
    // could add a map price -> limit to enable efficient finding of orders @ price.

    template<typename Sort>
    void AddOrder(Order order, long price, std::map<long, std::shared_ptr<Limit>, Sort> &limitLevels,
                  std::unordered_map<long, std::shared_ptr<OrderBookEntry>> &internalOrderBook);

    bool RemoveOrder(long orderId, const std::shared_ptr<OrderBookEntry> &obe);

public:
    OrderBook(const Security &instrument, mdfeed::MDAdapter<MarketDataPublisher> mdAdapter);

    size_t Count();

    bool ContainsOrder(long orderId);

    OrderBookSpread GetSpread();

    boost::optional<std::shared_ptr<Limit>> GetBestBidLimit();

    boost::optional<std::shared_ptr<Limit>> GetBestAskLimit();

    boost::optional<long> GetBestBidPrice();

    boost::optional<long> GetBestAskPrice();

    void PlaceMarketBuyOrder(uint32_t quantity);

    void PlaceMarketSellOrder(uint32_t quantity);

    void AddOrder(const Order &order);

    void AmendOrder(const long orderId, const Order &order);

    void RemoveOrder(const long orderId);

    std::list<OrderBookEntry> GetAskOrders();

    std::list<OrderBookEntry> GetBidOrders();

    std::map<long, uint32_t> GetBidQuantities();

    std::map<long, uint32_t> GetAskQuantities();

    std::list<OrderStruct> GetOrders();

    long GetOrdersMatched() const {
        return matchedQuantity_;
    }

    template<typename LimitMap>
    uint32_t TryMatch(Order &incomingOrder, long price, LimitMap &opposingLimits);
};
