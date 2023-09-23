#include <boost/beast/core.hpp>
#include <iostream>

#include "util.hpp"

namespace beast = boost::beast;

// Report a failure
void
fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}