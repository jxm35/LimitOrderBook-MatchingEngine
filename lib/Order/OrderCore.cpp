#include "OrderCore.h"

long OrderCore::ID = 0;

OrderCore::OrderCore(const std::string& username, int securityId) {
    orderId_ = ID++;
    username_ = username;
    securityId_ = securityId;

}

OrderCore::OrderCore(long orderId, const std::string& username, int securityId) {
    orderId_ = orderId;
    username_ = username;
    securityId_ = securityId;
}


