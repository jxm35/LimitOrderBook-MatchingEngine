#include <iostream>
#include "OrderBook.h"

int main(int argc, char **argv) {
    OrderBook book(Security("apple", "aapl", 1));
    std::cout << "working" << std::endl;

}