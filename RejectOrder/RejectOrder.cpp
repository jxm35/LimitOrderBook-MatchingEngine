
#include "RejectOrder.h"

RejectOrder::RejectOrder(OrderCore rejectedOrder, RejectionReason rejectionReason)
                : OrderCore(rejectedOrder) {
    rejectionReason_ = rejectionReason;
}
