#pragma once

#include "string"

class Security {
private:
    std::string name_;
    std::string ticker_;
    int securityId_;
public:
    Security(const std::string &&name, const std::string &&ticker, int securityId);

    [[nodiscard]] int GetSecurityId() const {
        return securityId_;
    }
};


