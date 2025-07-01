#include "securities/Security.h"

Security::Security(const std::string &&name, const std::string &&ticker, int securityId) {
    name_ = name;
    ticker_ = ticker;
    securityId_ = securityId;

}
