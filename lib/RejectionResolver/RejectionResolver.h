#pragma once

#include "OrderCore.h"
#include "RejectOrder.h"


class RejectionResolver final {
public:
    static RejectOrder GenerateOrderRejection(const OrderCore& rejectedOrder, RejectionReason rejectionReason) {
        return {rejectedOrder, rejectionReason};
    }
};
