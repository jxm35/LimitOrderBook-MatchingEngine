#pragma once

#include <boost/optional.hpp>
#include <list>
#include <unordered_map>
#include <set>
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
    virtual ~IReadOnlyOrderBook() { }
    virtual bool ContainsOrder(long orderId) = 0;
    virtual OrderBookSpread GetSpread() = 0;
    virtual int Count() = 0;
};

class IOrderEntryOrderBook: IReadOnlyOrderBook {
public:
    virtual ~IOrderEntryOrderBook() { }
    virtual void AddOrder(Order order) = 0;
    virtual void ChangeOrder(ModifyOrder modifyOrder) = 0;
    virtual void RemoveOrder(CancelOrder cancelOrder) = 0;
//    cancel all
};

class IRetrievalEntryOrderBook: IOrderEntryOrderBook {
public:
    virtual ~IRetrievalEntryOrderBook() { }
    virtual std::list<OrderBookEntry*> GetAskOrders() = 0;
    virtual std::list<OrderBookEntry> GetBidOrders() = 0;

};

class IMatchingOrderBook: IRetrievalEntryOrderBook {
public:
    virtual ~IMatchingOrderBook() { }
    virtual MatchResult Match() = 0;

};

class OrderBook: IRetrievalEntryOrderBook {
private:
    Security instrument_;
    // sorted sets
    std::set<Limit, SortAsks> askLimits_;
    std::set<Limit, SortBids> bidLimits_;
    // dictionary
    std::unordered_map<long, OrderBookEntry> orders_;

    static void AddOrderInner(Order order, Limit limit, std::set<Limit, SortAsks>& limitLevels, std::unordered_map<long, OrderBookEntry>& internalOrderBook);
    static void AddOrderInner(Order order, Limit limit, std::set<Limit, SortBids>& limitLevels, std::unordered_map<long, OrderBookEntry>& internalOrderBook);
    static void RemoveOrderInner(long orderId, OrderBookEntry *obe, std::unordered_map<long, OrderBookEntry> internalOrderBook);

public:
    OrderBook(Security instrument);
    int Count();
    bool ContainsOrder(long orderId);
    OrderBookSpread GetSpread();
    void AddOrder(Order order);
    void ChangeOrder(ModifyOrder modifyOrder);
    void RemoveOrder(CancelOrder cancelOrder) ;
    std::list<OrderBookEntry*> GetAskOrders();
    std::list<OrderBookEntry> GetBidOrders();

};
