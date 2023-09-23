#pragma once

#include "OrderCore.h"
#include "RejectOrder.h"


class RejectionResolver final {
public:
    static RejectOrder GenerateOrderRejection(OrderCore rejectedOrder, RejectionReason rejectionReason) {
        return RejectOrder(rejectedOrder, rejectionReason);
    }

};
