#pragma once

#include "http_client.hpp"

#include <string>
#include <vector>

namespace yzu {

class SelfService {
public:
    SelfService(CurlHttpClient& client, std::string user_agent);

    void login(const std::string& username, const std::string& password);
    std::vector<std::string> getOnlines();
    void logout(const std::string& name, const std::string& ip);

private:
    CurlHttpClient& client_;
    std::string user_agent_;
};

} // namespace yzu
