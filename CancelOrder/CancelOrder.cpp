#include "CancelOrder.h"

#include <utility>

CancelOrder::CancelOrder(OrderCore orderCore): OrderCore(std::move(orderCore)) {

}
