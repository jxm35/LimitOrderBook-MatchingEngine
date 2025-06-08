#pragma once

#include "CancelOrder.h"
#include "order.h"
#include "OrderStatus.h"


class OrderStatusResolver final {
public:
    static CancelOrderStatus GenerateCancelOrderStatus(const CancelOrder &cancelOrder) {
        return {};
    }

    static NewOrderStatus GenerateNewOrderStatus(const Order &order) {
        return {};
    }

    static ModifyOrderStatus GenerateModifyOrderStatus(const ModifyOrder &modifyOrder) {
        return {};
    }

};
