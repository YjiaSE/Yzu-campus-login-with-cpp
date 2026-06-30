#include "self_service.hpp"

#include "utils.hpp"

#include <regex>
#include <stdexcept>
#include <string>
#include <utility>

namespace yzu {
namespace {

const char* kServiceHost = "10.245.1.113:8080/selfservice/module";

} // namespace

SelfService::SelfService(CurlHttpClient& client, std::string user_agent)
    : client_(client), user_agent_(std::move(user_agent))
{
}

void SelfService::login(const std::string& username, const std::string& password)
{
    const auto body = formEncode(client_, {
        {"name", username},
        {"password", password},
    });

    const auto response = client_.post(
        std::string("http://") + kServiceHost + "/scgroup/web/login_judge.jsf?mobileslef=true",
        body,
        {
            "User-Agent: " + user_agent_,
            "Content-Type: application/x-www-form-urlencoded",
        });

    if (response.status_code < 200 || response.status_code >= 300) {
        throw std::runtime_error("error self service login: HTTP " + std::to_string(response.status_code));
    }

    const std::regex pattern(R"(errorMsg=(.*?)&)");
    std::smatch match;
    if (std::regex_search(response.body, match, pattern)) {
        throw std::runtime_error("Login error: " + match[1].str());
    }
}

std::vector<std::string> SelfService::getOnlines()
{
    const auto response = client_.get(
        std::string("http://") + kServiceHost + "/webcontent/web/onlinedevice_list.jsf",
        {
            "User-Agent: " + user_agent_,
        });

    if (response.status_code < 200 || response.status_code >= 300) {
        throw std::runtime_error("error get onlines: HTTP " + std::to_string(response.status_code));
    }

    if (contains(response.body, "操作执行失败")) {
        throw std::runtime_error("get onlines error: remote server error");
    }

    const std::regex pattern(R"regex(<input.*?id="userIp.*?value="(.*?)".*?>)regex");
    std::vector<std::string> result;
    for (auto it = std::sregex_iterator(response.body.begin(), response.body.end(), pattern);
         it != std::sregex_iterator();
         ++it) {
        result.push_back((*it)[1].str());
    }

    if (result.empty()) {
        throw std::runtime_error("get onlines error: cannot find ips");
    }

    return result;
}

void SelfService::logout(const std::string& name, const std::string& ip)
{
    const auto body = formEncode(client_, {
        {"key", name + ":" + ip},
    });

    const auto response = client_.post(
        std::string("http://") + kServiceHost + "/userself/web/userself_ajax.jsf?methodName=indexBean.kickUserBySelfForAjax",
        body,
        {
            "User-Agent: " + user_agent_,
            "Content-Type: application/x-www-form-urlencoded",
        });

    if (response.status_code < 200 || response.status_code >= 300) {
        throw std::runtime_error("error self service logout: HTTP " + std::to_string(response.status_code));
    }

    if (contains(response.body, "操作执行失败")) {
        throw std::runtime_error("logout error: remote server error");
    }
}

} // namespace yzu
