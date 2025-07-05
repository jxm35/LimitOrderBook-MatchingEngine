#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>

#include "orders/Order.h"
#include "orders/OrderCore.h"
#include "securities/Security.h"
#include "core/OrderBook.h"
#include "publisher/MarketDataPublisher.h"

namespace py = pybind11;

using PyOrderBook = OrderBook<mdfeed::NullMarketDataPublisher>;

PYBIND11_MODULE(_orderbook, m)
{
    m.doc() = "Python bindings for C++ OrderBook library";

    py::class_<Security>(m, "Security")
        .def(py::init([](const std::string& name, const std::string& ticker, int security_id)
             {
                 return Security(std::move(name), std::move(ticker), security_id);
             }), "Create a new Security",
             py::arg("name"), py::arg("ticker"), py::arg("security_id"))
        .def("get_security_id", &Security::GetSecurityId,
             "Get the security ID");

    py::class_<OrderCore>(m, "OrderCore")
        .def(py::init<const std::string&, int>(),
             "Create OrderCore with auto-generated ID",
             py::arg("username"), py::arg("security_id"))
        .def(py::init<long, const std::string&, int>(),
             "Create OrderCore with specific ID",
             py::arg("order_id"), py::arg("username"), py::arg("security_id"))
        .def("order_id", &OrderCore::OrderId, "Get order ID")
        .def("username", &OrderCore::Username, "Get username")
        .def("security_id", &OrderCore::SecurityID, "Get security ID");

    py::class_<Order>(m, "Order")
        .def(py::init<const OrderCore&, long, uint32_t, bool>(),
             "Create a new Order",
             py::arg("order_core"), py::arg("price"), py::arg("quantity"), py::arg("is_buy"))
        .def("price", &Order::Price, "Get order price")
        .def("initial_quantity", &Order::InitialQuantity, "Get initial quantity")
        .def("current_quantity", &Order::CurrentQuantity, "Get current quantity")
        .def("is_buy", &Order::IsBuy, "Check if this is a buy order")
        .def("order_id", &Order::OrderId, "Get order ID")
        .def("username", &Order::Username, "Get username")
        .def("security_id", &Order::SecurityID, "Get security ID");

    py::class_<OrderBookSpread>(m, "OrderBookSpread")
        .def("spread", [](OrderBookSpread& self) -> py::object
        {
            auto spread = self.Spread();
            if (spread.has_value())
            {
                return py::cast(spread.value());
            }
            return py::none();
        }, "Get the spread (None if no spread available)");

    py::class_<PyOrderBook>(m, "OrderBook")
        .def(py::init([](const Security& security)
        {
            static mdfeed::NullMarketDataPublisher publisher;
            static mdfeed::MDAdapter adapter(
                security.GetSecurityId(), publisher);
            return std::make_unique<PyOrderBook>(security, adapter);
        }), "Create a new OrderBook", py::arg("security"))

        .def("count", &PyOrderBook::Count, "Get total number of orders")
        .def("contains_order", &PyOrderBook::ContainsOrder,
             "Check if order exists", py::arg("order_id"))

        .def("get_spread", &PyOrderBook::GetSpread, "Get bid-ask spread")

        .def("get_best_bid_price", [](PyOrderBook& self) -> py::object
        {
            auto price = self.GetBestBidPrice();
            if (price.has_value())
            {
                return py::cast(price.value());
            }
            return py::none();
        }, "Get best bid price (None if no bids)")

        .def("get_best_ask_price", [](PyOrderBook& self) -> py::object
        {
            auto price = self.GetBestAskPrice();
            if (price.has_value())
            {
                return py::cast(price.value());
            }
            return py::none();
        }, "Get best ask price (None if no asks)")

        .def("place_market_buy_order", &PyOrderBook::PlaceMarketBuyOrder,
             "Place a market buy order", py::arg("quantity"))
        .def("place_market_sell_order", &PyOrderBook::PlaceMarketSellOrder,
             "Place a market sell order", py::arg("quantity"))

        .def("add_order", [](PyOrderBook& self, const Order& order)
        {
            self.AddOrder(order);
        }, "Add a limit order", py::arg("order"))

        .def("amend_order", [](PyOrderBook& self, long order_id, const Order& new_order)
        {
            self.AmendOrder(order_id, new_order);
        }, "Amend an existing order", py::arg("order_id"), py::arg("new_order"))

        .def("remove_order", [](PyOrderBook& self, long order_id)
        {
            self.RemoveOrder(order_id);
        }, "Remove an order", py::arg("order_id"))

        .def("get_bid_quantities", &PyOrderBook::GetBidQuantities,
             "Get bid quantities by price level")
        .def("get_ask_quantities", &PyOrderBook::GetAskQuantities,
             "Get ask quantities by price level")

        .def("get_orders_matched", &PyOrderBook::GetOrdersMatched,
             "Get total quantity of orders matched");

    m.def("create_order", [](const std::string& username, int security_id,
                             long price, uint32_t quantity, bool is_buy)
          {
              OrderCore core(username, security_id);
              return Order(core, price, quantity, is_buy);
          }, "Create an order with auto-generated ID",
          py::arg("username"), py::arg("security_id"), py::arg("price"),
          py::arg("quantity"), py::arg("is_buy"));
}
