#pragma once

#include "http_client.hpp"

#include <string>
#include <utility>

namespace yzu {

class CampusNet {
public:
    CampusNet(CurlHttpClient& client, std::string user_agent);

    std::pair<std::string, std::string> getHostQuery(const std::string& portal_url) const;
    std::string getPortalUrl();
    void loginService(const std::string& portal_url, const std::string& user_id, const std::string& service);
    void oldLoginService(
        const std::string& portal_url,
        const std::string& user_id,
        const std::string& password,
        const std::string& service);

private:
    CurlHttpClient& client_;
    std::string user_agent_;
};

} // namespace yzu
