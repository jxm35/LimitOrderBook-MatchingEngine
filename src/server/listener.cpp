#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <memory>

#include "listener.hpp"
#include "session.hpp"
#include "util.hpp"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


listener::listener(
        boost::asio::io_context &ioc,
        boost::asio::ip::tcp::endpoint endpoint)
        : ioc_(ioc), acceptor_(ioc) {
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if(ec)
    {
        fail(ec, "open");
        return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if(ec)
    {
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if(ec)
    {
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(
            net::socket_base::max_listen_connections, ec);
    if(ec)
    {
        fail(ec, "listen");
        return;
    }
}

void listener::run() {
    do_accept();
}

void listener::do_accept() {
    // The new connection gets its own strand
    acceptor_.async_accept(
            net::make_strand(ioc_),
            beast::bind_front_handler(
                    &listener::on_accept,
                    shared_from_this()));
}

void listener::on_accept(boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
    if(ec)
    {
        fail(ec, "accept");
    }
    else
    {
        // Create the session and run it
        std::make_shared<session>(std::move(socket))->run();
    }

    // Accept another connection
    do_accept();
}