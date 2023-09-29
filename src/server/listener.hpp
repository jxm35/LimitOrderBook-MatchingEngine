#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <memory>

class listener : public std::enable_shared_from_this<listener>
{
public:
    listener(
            boost::asio::io_context& ioc,
            boost::asio::ip::tcp::endpoint endpoint);

    // Start accepting incoming connections
    void run();

private:
    void do_accept();

    void on_accept(boost::system::error_code ec, boost::asio::ip::tcp::socket socket);

private:
    boost::asio::io_context& ioc_;
    boost::asio::ip::tcp::acceptor acceptor_;
};