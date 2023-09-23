#pragma once

#include "string"

class OrderCore {
private:
    static long ID;

    long orderId_;
    std::string username_;
    int securityId_;

public:
    OrderCore(std::string username, int securityId);
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


