#pragma once

#include "OrderCore.h"

enum RejectionReason {
    Unknown,
    OrderNotFound,
    InstrumentNotFound,
    IncorrectOrderSide
};


class RejectOrder : public OrderCore {
    RejectionReason rejectionReason_;
public:
    RejectOrder(const OrderCore &rejectedOrder, RejectionReason rejectionReason);

};


