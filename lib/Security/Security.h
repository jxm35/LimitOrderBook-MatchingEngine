#pragma once

#include "string"

class Security {
private:
    std::string name_;
    std::string ticker_;
    int securityId_;
public:
    Security(std::string name, std::string ticker, int securityId);

};


