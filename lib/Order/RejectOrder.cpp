
#include "RejectOrder.h"

RejectOrder::RejectOrder(const OrderCore& rejectedOrder, RejectionReason rejectionReason)
                : OrderCore(rejectedOrder) {
    rejectionReason_ = rejectionReason;
}
