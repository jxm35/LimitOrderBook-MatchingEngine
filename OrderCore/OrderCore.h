#pragma once

#include "string"

class OrderCore {
public:
    long orderId_;
    std::string username_;
    int securityId_;

public:
    OrderCore(long orderId, std::string username, int securityId);

    inline long OrderId() const {
        return orderId_;
    };
    inline std::string Username() const {
        return username_;
    }
    inline int SecurityID() const {
        return securityId_;
    }
};


