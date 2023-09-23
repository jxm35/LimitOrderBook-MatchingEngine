//
// Created by James Brennan on 20/09/2023.
//

#include "OrderCore.h"

OrderCore::OrderCore(long orderId, std::string username, int securityId) {
    orderId_ = orderId;
    username_ = username;
    securityId_ = securityId;
}
