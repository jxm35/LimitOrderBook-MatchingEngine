#include "messages/Messages.h"
#include "receiver/MulticastReceiver.h"
#include "receiver/ReceiverConfig.h"
#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(_mdfeed_receiver, m)
{
    m.doc() = "Python bindings for MDFeed MulticastReceiver library";

    // Enums
    py::enum_<mdfeed::MessageType>(m, "MDMessageType")
            .value("HEARTBEAT", mdfeed::MessageType::HEARTBEAT)
            .value("PRICE_LEVEL_UPDATE",
                   mdfeed::MessageType::PRICE_LEVEL_UPDATE)
            .value("PRICE_LEVEL_DELETE",
                   mdfeed::MessageType::PRICE_LEVEL_DELETE)
            .value("TRADE", mdfeed::MessageType::TRADE)
            .value("SNAPSHOT_BEGIN", mdfeed::MessageType::SNAPSHOT_BEGIN)
            .value("SNAPSHOT_ENTRY", mdfeed::MessageType::SNAPSHOT_ENTRY)
            .value("SNAPSHOT_END", mdfeed::MessageType::SNAPSHOT_END)
            .value("BOOK_CLEAR", mdfeed::MessageType::BOOK_CLEAR);

    py::enum_<mdfeed::Side>(m, "MDSide")
            .value("BUY", mdfeed::Side::BUY)
            .value("SELL", mdfeed::Side::SELL);

    py::enum_<mdfeed::UpdateAction>(m, "UpdateAction")
            .value("NEW", mdfeed::UpdateAction::NEW)
            .value("CHANGE", mdfeed::UpdateAction::CHANGE)
            .value("DELETE", mdfeed::UpdateAction::DELETE);

    // MessageHeader
    py::class_<mdfeed::MessageHeader>(m, "MessageHeader")
            .def_readonly("sequence_number",
                          &mdfeed::MessageHeader::sequence_number)
            .def_readonly("message_length",
                          &mdfeed::MessageHeader::message_length)
            .def_readonly("message_type", &mdfeed::MessageHeader::message_type)
            .def_readonly("timestamp_ns", &mdfeed::MessageHeader::timestamp_ns)
            .def_readonly("instrument_id",
                          &mdfeed::MessageHeader::instrument_id)
            .def("to_debug_string", &mdfeed::MessageHeader::toDebugString);

    // Message types
    py::class_<mdfeed::HeartbeatMessage>(m, "HeartbeatMessage")
            .def_readonly("header", &mdfeed::HeartbeatMessage::header)
            .def("to_debug_string", &mdfeed::HeartbeatMessage::toDebugString);

    py::class_<mdfeed::PriceLevelUpdateMessage>(m, "PriceLevelUpdateMessage")
            .def_readonly("header", &mdfeed::PriceLevelUpdateMessage::header)
            .def_readonly("price", &mdfeed::PriceLevelUpdateMessage::price)
            .def_readonly("quantity",
                          &mdfeed::PriceLevelUpdateMessage::quantity)
            .def_readonly("side", &mdfeed::PriceLevelUpdateMessage::side)
            .def_readonly("action", &mdfeed::PriceLevelUpdateMessage::action)
            .def("to_debug_string",
                 &mdfeed::PriceLevelUpdateMessage::toDebugString);

    py::class_<mdfeed::PriceLevelDeleteMessage>(m, "PriceLevelDeleteMessage")
            .def_readonly("header", &mdfeed::PriceLevelDeleteMessage::header)
            .def_readonly("price", &mdfeed::PriceLevelDeleteMessage::price)
            .def_readonly("side", &mdfeed::PriceLevelDeleteMessage::side)
            .def("to_debug_string",
                 &mdfeed::PriceLevelDeleteMessage::toDebugString);

    py::class_<mdfeed::TradeMessage>(m, "TradeMessage")
            .def_readonly("header", &mdfeed::TradeMessage::header)
            .def_readonly("trade_id", &mdfeed::TradeMessage::trade_id)
            .def_readonly("price", &mdfeed::TradeMessage::price)
            .def_readonly("quantity", &mdfeed::TradeMessage::quantity)
            .def_readonly("aggressor_side",
                          &mdfeed::TradeMessage::aggressor_side)
            .def("to_debug_string", &mdfeed::TradeMessage::toDebugString);

    py::class_<mdfeed::SnapshotBeginMessage>(m, "SnapshotBeginMessage")
            .def_readonly("header", &mdfeed::SnapshotBeginMessage::header)
            .def_readonly("total_entries",
                          &mdfeed::SnapshotBeginMessage::total_entries)
            .def("to_debug_string",
                 &mdfeed::SnapshotBeginMessage::toDebugString);

    py::class_<mdfeed::SnapshotEntryMessage>(m, "SnapshotEntryMessage")
            .def_readonly("header", &mdfeed::SnapshotEntryMessage::header)
            .def_readonly("price", &mdfeed::SnapshotEntryMessage::price)
            .def_readonly("quantity", &mdfeed::SnapshotEntryMessage::quantity)
            .def_readonly("side", &mdfeed::SnapshotEntryMessage::side)
            .def("to_debug_string",
                 &mdfeed::SnapshotEntryMessage::toDebugString);

    py::class_<mdfeed::SnapshotEndMessage>(m, "SnapshotEndMessage")
            .def_readonly("header", &mdfeed::SnapshotEndMessage::header)
            .def_readonly("checksum", &mdfeed::SnapshotEndMessage::checksum)
            .def("to_debug_string", &mdfeed::SnapshotEndMessage::toDebugString);

    py::class_<mdfeed::BookClearMessage>(m, "BookClearMessage")
            .def_readonly("header", &mdfeed::BookClearMessage::header)
            .def_readonly("reason_code", &mdfeed::BookClearMessage::reason_code)
            .def("to_debug_string", &mdfeed::BookClearMessage::toDebugString);

    // ReceiverConfig
    py::class_<mdfeed::ReceiverConfig>(m, "ReceiverConfig")
            .def(py::init<>())
            .def_readwrite("multicast_ip",
                           &mdfeed::ReceiverConfig::multicast_ip)
            .def_readwrite("multicast_port",
                           &mdfeed::ReceiverConfig::multicast_port)
            .def_readwrite("interface_ip",
                           &mdfeed::ReceiverConfig::interface_ip)
            .def_readwrite("receive_buffer_size",
                           &mdfeed::ReceiverConfig::receive_buffer_size)
            .def_readwrite("enable_logging",
                           &mdfeed::ReceiverConfig::enable_logging)
            .def_readwrite("log_to_console",
                           &mdfeed::ReceiverConfig::log_to_console)
            .def_readwrite("log_file_path",
                           &mdfeed::ReceiverConfig::log_file_path)
            .def_readwrite("validate_sequence_numbers",
                           &mdfeed::ReceiverConfig::validate_sequence_numbers)
            .def_readwrite("stats_interval",
                           &mdfeed::ReceiverConfig::stats_interval);

    // Stats
    py::class_<mdfeed::MulticastReceiver::Stats>(m, "ReceiverStats")
            .def_readonly(
                    "total_messages_received",
                    &mdfeed::MulticastReceiver::Stats::total_messages_received)
            .def_readonly(
                    "total_bytes_received",
                    &mdfeed::MulticastReceiver::Stats::total_bytes_received)
            .def_readonly("sequence_gaps",
                          &mdfeed::MulticastReceiver::Stats::sequence_gaps)
            .def_readonly("invalid_messages",
                          &mdfeed::MulticastReceiver::Stats::invalid_messages)
            .def_readonly("start_time",
                          &mdfeed::MulticastReceiver::Stats::start_time);

    // MessageHandler typedef
    py::class_<mdfeed::MulticastReceiver>(m, "MulticastReceiver")
            .def(py::init<>())
            .def(py::init<const mdfeed::ReceiverConfig&>())
            .def("initialize", &mdfeed::MulticastReceiver::initialize,
                 "Initialize receiver with config", py::arg("config"))
            .def("start", &mdfeed::MulticastReceiver::start,
                 "Start the receiver")
            .def("stop", &mdfeed::MulticastReceiver::stop, "Stop the receiver")
            .def("is_running", &mdfeed::MulticastReceiver::is_running,
                 "Check if receiver is running")
            .def("set_message_handler",
                 &mdfeed::MulticastReceiver::set_message_handler,
                 "Set custom message handler function", py::arg("handler"))
            .def("get_stats", &mdfeed::MulticastReceiver::get_stats,
                 "Get receiver statistics")
            .def("reset_stats", &mdfeed::MulticastReceiver::reset_stats,
                 "Reset receiver statistics");

    // Helper function to cast message data based on type
    m.def(
            "cast_md_message",
            [](const mdfeed::MessageHeader& header,
               py::bytes data) -> py::object {
                std::string data_str = data;
                const void* data_ptr = data_str.data();

                switch (static_cast<mdfeed::MessageType>(header.message_type)) {
                case mdfeed::MessageType::HEARTBEAT:
                    return py::cast(
                            *reinterpret_cast<const mdfeed::HeartbeatMessage*>(
                                    data_ptr));
                case mdfeed::MessageType::PRICE_LEVEL_UPDATE:
                    return py::cast(*reinterpret_cast<
                                    const mdfeed::PriceLevelUpdateMessage*>(
                            data_ptr));
                case mdfeed::MessageType::PRICE_LEVEL_DELETE:
                    return py::cast(*reinterpret_cast<
                                    const mdfeed::PriceLevelDeleteMessage*>(
                            data_ptr));
                case mdfeed::MessageType::TRADE:
                    return py::cast(
                            *reinterpret_cast<const mdfeed::TradeMessage*>(
                                    data_ptr));
                case mdfeed::MessageType::SNAPSHOT_BEGIN:
                    return py::cast(*reinterpret_cast<
                                    const mdfeed::SnapshotBeginMessage*>(
                            data_ptr));
                case mdfeed::MessageType::SNAPSHOT_ENTRY:
                    return py::cast(*reinterpret_cast<
                                    const mdfeed::SnapshotEntryMessage*>(
                            data_ptr));
                case mdfeed::MessageType::SNAPSHOT_END:
                    return py::cast(*reinterpret_cast<
                                    const mdfeed::SnapshotEndMessage*>(
                            data_ptr));
                case mdfeed::MessageType::BOOK_CLEAR:
                    return py::cast(
                            *reinterpret_cast<const mdfeed::BookClearMessage*>(
                                    data_ptr));
                default:
                    return py::none();
                }
            },
            "Cast raw message data to appropriate market data message type",
            py::arg("header"), py::arg("data"));

    // Utility functions
    m.def("get_md_timestamp_ns", &mdfeed::message_utils::get_timestamp_ns,
          "Get current timestamp in nanoseconds");

    m.def("format_md_timestamp", &mdfeed::message_utils::format_timestamp,
          "Format timestamp to readable string", py::arg("timestamp_ns"));

    m.def("md_message_type_to_string",
          &mdfeed::message_utils::message_type_to_string,
          "Convert market data message type enum to string", py::arg("type"));
}
