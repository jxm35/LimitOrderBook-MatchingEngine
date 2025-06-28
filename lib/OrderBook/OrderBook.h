#pragma once

#include <boost/optional.hpp>
#include <list>
#include <unordered_map>
#include <map>


#include "order.h"
#include "OrderBookEntry.h"
#include "MatchResult.h"
#include "Security.h"
#include "publisher/DeltaGenerator.h"
#include "publisher/MDAdapter.h"

class OrderBookSpread
{
private:
    boost::optional<long> bid_;
    boost::optional<long> ask_;

public:
    OrderBookSpread(boost::optional<long> bid, boost::optional<long> ask)
    {
        bid_ = bid;
        ask_ = ask;
    }

    boost::optional<long> Spread()
    {
        if (bid_.has_value() && ask_.has_value())
        {
            return ask_.value() - bid_.value();
        }
        return boost::none;
    }
};

class OrderBook
{
private:
    Security instrument_;
    long ordersMatched_;
    std::unique_ptr<mdfeed::MDAdapter> md_adapter_;

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

    template <typename sort>
    static void AddOrder(Order order, long price, std::map<long, std::shared_ptr<Limit>, sort>& limitLevels,
                         std::unordered_map<long, std::shared_ptr<OrderBookEntry>>& internalOrderBook);

    template <typename sort>
    static void
    RemoveOrder(long orderId, const std::shared_ptr<OrderBookEntry>& obe,
                std::map<long, std::shared_ptr<Limit>, sort>& limitLevels,
                std::unordered_map<long, std::shared_ptr<OrderBookEntry>>& internalOrderBook);

public:
    OrderBook(const Security& instrument, std::unique_ptr<mdfeed::DeltaGenerator>);

    size_t Count();

    bool ContainsOrder(long orderId);

    OrderBookSpread GetSpread();

    boost::optional<std::shared_ptr<Limit>> GetBestBidLimit();

    boost::optional<std::shared_ptr<Limit>> GetBestAskLimit();

    boost::optional<long> GetBestBidPrice();

    boost::optional<long> GetBestAskPrice();

    void PlaceMarketBuyOrder(uint32_t quantity);

    void PlaceMarketSellOrder(uint32_t quantity);

    void AddOrder(const Order& order);

    void ChangeOrder(ModifyOrder modifyOrder);

    void RemoveOrder(const CancelOrder& cancelOrder);

    std::list<OrderBookEntry> GetAskOrders();

    std::list<OrderBookEntry> GetBidOrders();

    std::map<long, uint32_t> GetBidQuantities();

    std::map<long, uint32_t> GetAskQuantities();

    std::list<OrderStruct> GetOrders();

    long GetOrdersMatched() const
    {
        return ordersMatched_;
    }

    MatchResult Match();

    void set_market_data_generator(std::unique_ptr<mdfeed::DeltaGenerator> generator)
    {
        if (md_adapter_)
        {
            md_adapter_->set_delta_generator(std::move(generator));
        }
    }
};
