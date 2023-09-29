#pragma once

#include <boost/beast/core/error.hpp>
#include <iostream>

void fail(boost::beast::error_code ec, const char* what);