#pragma once

#include <curl/curl.h>

#include <string>
#include <utility>
#include <vector>

namespace yzu {

struct HttpResponse {
    long status_code{0};
    std::string body;
    std::string effective_url;
};

class CurlGlobal {
public:
    CurlGlobal();
    ~CurlGlobal();

    CurlGlobal(const CurlGlobal&) = delete;
    CurlGlobal& operator=(const CurlGlobal&) = delete;
};

class CurlHttpClient {
public:
    explicit CurlHttpClient(long timeout_seconds = 5);
    ~CurlHttpClient();

    CurlHttpClient(const CurlHttpClient&) = delete;
    CurlHttpClient& operator=(const CurlHttpClient&) = delete;

    HttpResponse get(const std::string& url, const std::vector<std::string>& headers = {});
    HttpResponse post(
        const std::string& url,
        const std::string& body,
        const std::vector<std::string>& headers = {});

    std::string escape(const std::string& value) const;

private:
    HttpResponse request(
        const std::string& method,
        const std::string& url,
        const std::string& body,
        const std::vector<std::string>& headers);

    CURL* curl_{nullptr};
    long timeout_seconds_{5};
};

std::string formEncode(
    CurlHttpClient& client,
    const std::vector<std::pair<std::string, std::string>>& values);

} // namespace yzu
