# pragma once

#include <boost/beast/core.hpp>
#include <boost/asio/strand.hpp>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "listener.hpp"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// Echoes back all received WebSocket messages
class session : public std::enable_shared_from_this<session>
{
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;

public:
    // Take ownership of the socket
    explicit
    session(tcp::socket&& socket)
            : ws_(std::move(socket))
    {
    }

    // Get on the correct executor
    void run();

    // Start the asynchronous operation
    void on_run();

    void on_accept(beast::error_code ec);

    void do_read();

    void on_read(
            beast::error_code ec,
            std::size_t bytes_transferred);

    void on_write(
            beast::error_code ec,
            std::size_t bytes_transferred);
};
