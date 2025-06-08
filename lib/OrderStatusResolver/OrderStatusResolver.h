#pragma once

#include "CancelOrder.h"
#include "order.h"
#include "OrderStatus.h"


class OrderStatusResolver final {
public:
    static CancelOrderStatus GenerateCancelOrderStatus(CancelOrder cancelOrder) {
        return {};
    }

    static NewOrderStatus GenerateNewOrderStatus(Order order) {
        return {};
    }

    static ModifyOrderStatus GenerateModifyOrderStatus(ModifyOrder modifyOrder) {
        return {};
    }

};
