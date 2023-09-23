#pragma once

#include <boost/optional.hpp>
#include <list>
#include <unordered_map>
#include <map>

#include "order.h"
#include "OrderBookEntry.h"
#include "MatchResult.h"
#include "Security.h"

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

class IReadOnlyOrderBook {
public:
    virtual ~IReadOnlyOrderBook() {}

    virtual bool ContainsOrder(long orderId) = 0;

    virtual OrderBookSpread GetSpread() = 0;

    virtual int Count() = 0;
};

class IOrderEntryOrderBook : IReadOnlyOrderBook {
public:
    virtual ~IOrderEntryOrderBook() {}

    virtual void AddOrder(Order order) = 0;

    virtual void ChangeOrder(ModifyOrder modifyOrder) = 0;

    virtual void RemoveOrder(CancelOrder cancelOrder) = 0;
//    cancel all
};

class IRetrievalEntryOrderBook : IOrderEntryOrderBook {
public:
    virtual ~IRetrievalEntryOrderBook() {}

    virtual std::list<OrderBookEntry> GetAskOrders() = 0;

    virtual std::list<OrderBookEntry> GetBidOrders() = 0;

};

class IMatchingOrderBook : IRetrievalEntryOrderBook {
public:
    virtual ~IMatchingOrderBook() {}

    virtual MatchResult Match() = 0;

};

class OrderBook : IRetrievalEntryOrderBook {
private:
    Security instrument_;

    // sorted maps
    std::map<long, Limit *, std::less<>> askLimits_;
    std::map<long, Limit *, std::greater<>> bidLimits_;

    // dictionary
    std::unordered_map<long, OrderBookEntry> orders_;

    template<typename sort>
    static void AddOrder(Order order, long price, std::map<long, Limit *, sort> &limitLevels,
                         std::unordered_map<long, OrderBookEntry> &internalOrderBook);

    static void
    RemoveOrderInner(long orderId, OrderBookEntry *obe, std::unordered_map<long, OrderBookEntry> &internalOrderBook);

public:
    OrderBook(Security instrument);

    int Count();

    bool ContainsOrder(long orderId);

    OrderBookSpread GetSpread();

    void AddOrder(Order order);

    void ChangeOrder(ModifyOrder modifyOrder);

    void RemoveOrder(CancelOrder cancelOrder);

    std::list<OrderBookEntry> GetAskOrders();

    std::list<OrderBookEntry> GetBidOrders();

};
