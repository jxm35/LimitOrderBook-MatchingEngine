
#include <boost/beast/core.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "session.hpp"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

int main(int argc, char** argv)
{
    // Check command line arguments.
    if(argc != 4)
    {
        std::cerr <<
                  "Usage: websocket-client-async <host> <port> <text>\n" <<
                  "Example:\n" <<
                  "    websocket-client-async echo.websocket.org 80 \"Hello, world!\"\n";
        return EXIT_FAILURE;
    }
    auto const host = argv[1];
    auto const port = argv[2];
    auto const text = argv[3];

    // The io_context is required for all I/O
    net::io_context ioc;

    // Launch the asynchronous operation
    std::make_shared<session>(ioc)->run(host, port, text);

    // Run the I/O service. The call will return when
    // the socket is closed.
    ioc.run();

    return EXIT_SUCCESS;
}