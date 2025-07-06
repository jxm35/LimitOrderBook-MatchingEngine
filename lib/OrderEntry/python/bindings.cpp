#include "client/OrderEntryClient.h"
#include "messages/OrderMessages.h"
#include <pybind11/chrono.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(_orderentry_client, m)
{
    m.doc() = "Python bindings for OrderEntry Client library";

    // Enums
    py::enum_<orderentry::MessageType>(m, "OEMessageType")
            .value("NEW_ORDER", orderentry::MessageType::NEW_ORDER)
            .value("CANCEL_ORDER", orderentry::MessageType::CANCEL_ORDER)
            .value("MODIFY_ORDER", orderentry::MessageType::MODIFY_ORDER)
            .value("ORDER_ACK", orderentry::MessageType::ORDER_ACK)
            .value("ORDER_REJECT", orderentry::MessageType::ORDER_REJECT)
            .value("EXECUTION_REPORT",
                   orderentry::MessageType::EXECUTION_REPORT);

    py::enum_<orderentry::Side>(m, "OESide")
            .value("BUY", orderentry::Side::BUY)
            .value("SELL", orderentry::Side::SELL);

    py::enum_<orderentry::OrderType>(m, "OrderType")
            .value("MARKET", orderentry::OrderType::MARKET)
            .value("LIMIT", orderentry::OrderType::LIMIT);

    py::enum_<orderentry::TimeInForce>(m, "TimeInForce")
            .value("DAY", orderentry::TimeInForce::DAY)
            .value("IOC", orderentry::TimeInForce::IOC);

    // MessageHeader
    py::class_<orderentry::MessageHeader>(m, "MessageHeader")
            .def_readonly("sequence_number",
                          &orderentry::MessageHeader::sequence_number)
            .def_readonly("message_length",
                          &orderentry::MessageHeader::message_length)
            .def_readonly("message_type",
                          &orderentry::MessageHeader::message_type)
            .def_readonly("timestamp_ns",
                          &orderentry::MessageHeader::timestamp_ns)
            .def_readonly("instrument_id",
                          &orderentry::MessageHeader::instrument_id)
            .def("to_debug_string", &orderentry::MessageHeader::toDebugString);

    // Message types
    py::class_<orderentry::NewOrderMessage>(m, "NewOrderMessage")
            .def_readonly("header", &orderentry::NewOrderMessage::header)
            .def_readonly("client_order_id",
                          &orderentry::NewOrderMessage::client_order_id)
            .def_readonly("price", &orderentry::NewOrderMessage::price)
            .def_readonly("quantity", &orderentry::NewOrderMessage::quantity)
            .def_readonly("side", &orderentry::NewOrderMessage::side)
            .def_readonly("order_type",
                          &orderentry::NewOrderMessage::order_type)
            .def_readonly("time_in_force",
                          &orderentry::NewOrderMessage::time_in_force)
            .def_property_readonly(
                    "symbol",
                    [](const orderentry::NewOrderMessage& msg) {
                        return std::string(
                                msg.symbol,
                                strnlen(msg.symbol, sizeof(msg.symbol)));
                    })
            .def("to_debug_string",
                 &orderentry::NewOrderMessage::toDebugString);

    py::class_<orderentry::CancelOrderMessage>(m, "CancelOrderMessage")
            .def_readonly("header", &orderentry::CancelOrderMessage::header)
            .def_readonly("client_order_id",
                          &orderentry::CancelOrderMessage::client_order_id)
            .def_readonly(
                    "original_client_order_id",
                    &orderentry::CancelOrderMessage::original_client_order_id)
            .def_property_readonly(
                    "symbol",
                    [](const orderentry::CancelOrderMessage& msg) {
                        return std::string(
                                msg.symbol,
                                strnlen(msg.symbol, sizeof(msg.symbol)));
                    })
            .def("to_debug_string",
                 &orderentry::CancelOrderMessage::toDebugString);

    py::class_<orderentry::ModifyOrderMessage>(m, "ModifyOrderMessage")
            .def_readonly("header", &orderentry::ModifyOrderMessage::header)
            .def_readonly("client_order_id",
                          &orderentry::ModifyOrderMessage::client_order_id)
            .def_readonly(
                    "original_client_order_id",
                    &orderentry::ModifyOrderMessage::original_client_order_id)
            .def_readonly("new_price",
                          &orderentry::ModifyOrderMessage::new_price)
            .def_readonly("new_quantity",
                          &orderentry::ModifyOrderMessage::new_quantity)
            .def_property_readonly(
                    "symbol",
                    [](const orderentry::ModifyOrderMessage& msg) {
                        return std::string(
                                msg.symbol,
                                strnlen(msg.symbol, sizeof(msg.symbol)));
                    })
            .def("to_debug_string",
                 &orderentry::ModifyOrderMessage::toDebugString);

    py::class_<orderentry::OrderAckMessage>(m, "OrderAckMessage")
            .def_readonly("header", &orderentry::OrderAckMessage::header)
            .def_readonly("client_order_id",
                          &orderentry::OrderAckMessage::client_order_id)
            .def_readonly("exchange_order_id",
                          &orderentry::OrderAckMessage::exchange_order_id)
            .def_property_readonly(
                    "symbol",
                    [](const orderentry::OrderAckMessage& msg) {
                        return std::string(
                                msg.symbol,
                                strnlen(msg.symbol, sizeof(msg.symbol)));
                    })
            .def("to_debug_string",
                 &orderentry::OrderAckMessage::toDebugString);

    py::class_<orderentry::OrderRejectMessage>(m, "OrderRejectMessage")
            .def_readonly("header", &orderentry::OrderRejectMessage::header)
            .def_readonly("client_order_id",
                          &orderentry::OrderRejectMessage::client_order_id)
            .def_readonly("reject_reason",
                          &orderentry::OrderRejectMessage::reject_reason)
            .def_property_readonly(
                    "reject_text",
                    [](const orderentry::OrderRejectMessage& msg) {
                        return std::string(msg.reject_text,
                                           strnlen(msg.reject_text,
                                                   sizeof(msg.reject_text)));
                    })
            .def_property_readonly(
                    "symbol",
                    [](const orderentry::OrderRejectMessage& msg) {
                        return std::string(
                                msg.symbol,
                                strnlen(msg.symbol, sizeof(msg.symbol)));
                    })
            .def("to_debug_string",
                 &orderentry::OrderRejectMessage::toDebugString);

    py::class_<orderentry::ExecutionReportMessage>(m, "ExecutionReportMessage")
            .def_readonly("header", &orderentry::ExecutionReportMessage::header)
            .def_readonly("client_order_id",
                          &orderentry::ExecutionReportMessage::client_order_id)
            .def_readonly(
                    "exchange_order_id",
                    &orderentry::ExecutionReportMessage::exchange_order_id)
            .def_readonly("execution_id",
                          &orderentry::ExecutionReportMessage::execution_id)
            .def_readonly("execution_price",
                          &orderentry::ExecutionReportMessage::execution_price)
            .def_readonly(
                    "execution_quantity",
                    &orderentry::ExecutionReportMessage::execution_quantity)
            .def_readonly("leaves_quantity",
                          &orderentry::ExecutionReportMessage::leaves_quantity)
            .def_readonly("side", &orderentry::ExecutionReportMessage::side)
            .def("to_debug_string",
                 &orderentry::ExecutionReportMessage::toDebugString);

    // OrderEntryClient
    py::class_<orderentry::OrderEntryClient>(m, "OrderEntryClient")
            .def(py::init<>())
            .def("connect", &orderentry::OrderEntryClient::connect,
                 "Connect to order entry server", py::arg("server_address"),
                 py::arg("port"))
            .def("disconnect", &orderentry::OrderEntryClient::disconnect,
                 "Disconnect from server")
            .def("is_connected", &orderentry::OrderEntryClient::is_connected,
                 "Check if client is connected")
            .def("send_new_order",
                 &orderentry::OrderEntryClient::send_new_order,
                 "Send a new order", py::arg("client_order_id"),
                 py::arg("symbol"), py::arg("side"), py::arg("price"),
                 py::arg("quantity"))
            .def("send_cancel_order",
                 &orderentry::OrderEntryClient::send_cancel_order,
                 "Send a cancel order", py::arg("client_order_id"),
                 py::arg("original_order_id"), py::arg("symbol"));

    // Utility functions
    m.def("get_oe_timestamp_ns", &orderentry::message_utils::get_timestamp_ns,
          "Get current timestamp in nanoseconds");

    m.def("format_oe_timestamp", &orderentry::message_utils::format_timestamp,
          "Format timestamp to readable string", py::arg("timestamp_ns"));

    m.def("oe_message_type_to_string",
          &orderentry::message_utils::message_type_to_string,
          "Convert order entry message type enum to string", py::arg("type"));

    // Helper functions for easy order creation
    m.def(
            "create_new_order",
            [](uint64_t client_order_id, const std::string& symbol,
               orderentry::Side side, uint64_t price, uint64_t quantity) {
                orderentry::NewOrderMessage msg{};
                orderentry::message_utils::init_header(
                        msg, orderentry::MessageType::NEW_ORDER, 1, 1);
                msg.client_order_id = client_order_id;
                msg.price = price;
                msg.quantity = quantity;
                msg.side = side;
                msg.order_type = orderentry::OrderType::LIMIT;
                msg.time_in_force = orderentry::TimeInForce::DAY;

                size_t copy_len = std::min(symbol.length(), sizeof(msg.symbol));
                std::memcpy(msg.symbol, symbol.c_str(), copy_len);
                if (copy_len < sizeof(msg.symbol)) {
                    std::memset(msg.symbol + copy_len, 0,
                                sizeof(msg.symbol) - copy_len);
                }
                std::memset(msg.reserved, 0, sizeof(msg.reserved));

                return msg;
            },
            "Create a new order message", py::arg("client_order_id"),
            py::arg("symbol"), py::arg("side"), py::arg("price"),
            py::arg("quantity"));

    m.def(
            "create_cancel_order",
            [](uint64_t client_order_id, uint64_t original_order_id,
               const std::string& symbol) {
                orderentry::CancelOrderMessage msg{};
                orderentry::message_utils::init_header(
                        msg, orderentry::MessageType::CANCEL_ORDER, 1, 1);
                msg.client_order_id = client_order_id;
                msg.original_client_order_id = original_order_id;

                size_t copy_len = std::min(symbol.length(), sizeof(msg.symbol));
                std::memcpy(msg.symbol, symbol.c_str(), copy_len);
                if (copy_len < sizeof(msg.symbol)) {
                    std::memset(msg.symbol + copy_len, 0,
                                sizeof(msg.symbol) - copy_len);
                }
                std::memset(msg.reserved, 0, sizeof(msg.reserved));

                return msg;
            },
            "Create a cancel order message", py::arg("client_order_id"),
            py::arg("original_order_id"), py::arg("symbol"));
}
