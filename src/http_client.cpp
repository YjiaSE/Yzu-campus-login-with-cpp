#include "http_client.hpp"

#include <stdexcept>

namespace yzu {
namespace {

std::size_t writeBody(char* data, std::size_t size, std::size_t nmemb, void* user_data)
{
    const auto bytes = size * nmemb;
    auto* body = static_cast<std::string*>(user_data);
    body->append(data, bytes);
    return bytes;
}

} // namespace

CurlGlobal::CurlGlobal()
{
    const auto code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (code != CURLE_OK) {
        throw std::runtime_error("curl_global_init failed");
    }
}

CurlGlobal::~CurlGlobal()
{
    curl_global_cleanup();
}

CurlHttpClient::CurlHttpClient(long timeout_seconds)
    : timeout_seconds_(timeout_seconds)
{
    curl_ = curl_easy_init();
    if (curl_ == nullptr) {
        throw std::runtime_error("curl_easy_init failed");
    }
}

CurlHttpClient::~CurlHttpClient()
{
    if (curl_ != nullptr) {
        curl_easy_cleanup(curl_);
    }
}

HttpResponse CurlHttpClient::get(const std::string& url, const std::vector<std::string>& headers)
{
    return request("GET", url, "", headers);
}

HttpResponse CurlHttpClient::post(
    const std::string& url,
    const std::string& body,
    const std::vector<std::string>& headers)
{
    return request("POST", url, body, headers);
}

std::string CurlHttpClient::escape(const std::string& value) const
{
    char* escaped = curl_easy_escape(curl_, value.c_str(), static_cast<int>(value.size()));
    if (escaped == nullptr) {
        throw std::runtime_error("curl_easy_escape failed");
    }

    std::string result(escaped);
    curl_free(escaped);
    return result;
}

HttpResponse CurlHttpClient::request(
    const std::string& method,
    const std::string& url,
    const std::string& body,
    const std::vector<std::string>& headers)
{
    curl_easy_reset(curl_);

    std::string response_body;
    curl_slist* header_list = nullptr;
    for (const auto& header : headers) {
        header_list = curl_slist_append(header_list, header.c_str());
    }

    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, timeout_seconds_);
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl_, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeBody);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L);

    if (header_list != nullptr) {
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, header_list);
    }

    if (method == "POST") {
        curl_easy_setopt(curl_, CURLOPT_POST, 1L);
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    }

    const auto code = curl_easy_perform(curl_);
    if (header_list != nullptr) {
        curl_slist_free_all(header_list);
    }
    if (code != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(code));
    }

    HttpResponse response;
    response.body = std::move(response_body);
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response.status_code);

    char* effective_url = nullptr;
    curl_easy_getinfo(curl_, CURLINFO_EFFECTIVE_URL, &effective_url);
    if (effective_url != nullptr) {
        response.effective_url = effective_url;
    }

    return response;
}

std::string formEncode(
    CurlHttpClient& client,
    const std::vector<std::pair<std::string, std::string>>& values)
{
    std::string body;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            body += '&';
        }
        body += client.escape(values[i].first);
        body += '=';
        body += client.escape(values[i].second);
    }
    return body;
}

} // namespace yzu
