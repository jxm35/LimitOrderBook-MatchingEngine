#pragma once

#include "CancelOrder.h"
#include "order.h"
#include "OrderStatus.h"


class OrderStatusResolver final {
public:
    static CancelOrderStatus GenerateCancelOrderStatus(CancelOrder co) {
        return CancelOrderStatus();
    }
    static NewOrderStatus GenerateNewOrderStatus(Order o) {
        return NewOrderStatus();
    }
    static ModifyOrderStatus GenerateModifyOrderStatus(ModifyOrder mo) {
        return ModifyOrderStatus();
    }

};
