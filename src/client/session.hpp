# pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// Sends a WebSocket message and prints the response
class session : public std::enable_shared_from_this<session>
{
    tcp::resolver resolver_;
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string text_;

public:
    // Resolver and socket require an io_context
    explicit
    session(net::io_context& ioc)
            : resolver_(net::make_strand(ioc))
            , ws_(net::make_strand(ioc))
    {
    }

    // Start the asynchronous operation
    void run(
            char const* host,
            char const* port,
            char const* text);

    void on_resolve(
            beast::error_code ec,
            tcp::resolver::results_type results);

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);

    void on_handshake(beast::error_code ec);

    void on_write(
            beast::error_code ec,
            std::size_t bytes_transferred);

    void on_read(
            beast::error_code ec,
            std::size_t bytes_transferred);

    void on_close(beast::error_code ec);
};