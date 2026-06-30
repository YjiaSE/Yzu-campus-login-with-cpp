#include "campus_net.hpp"
#include "config.hpp"
#include "http_client.hpp"
#include "self_service.hpp"

#include <chrono>
#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {

void logInfo(const std::string& message)
{
    std::cout << message << std::endl;
}

void logError(const std::string& message)
{
    std::cerr << message << std::endl;
}

bool testConnection()
{
    try {
        yzu::CurlHttpClient client;
        const auto response = client.get("http://111.13.141.31/generate_204", {
            "Host: connect.rom.miui.com",
        });
        return response.status_code == 204;
    } catch (const std::exception&) {
        return false;
    }
}

void logout(const yzu::Config& config)
{
    yzu::CurlHttpClient client;
    yzu::SelfService self_service(client, config.user_agent);

    self_service.login(config.sso_username, config.sso_password);
    logInfo("Login success");

    const auto onlines = self_service.getOnlines();
    for (const auto& online : onlines) {
        self_service.logout(config.sso_username, online);
        logInfo("Logout " + online + " success");
    }
}

void loginLoop(const yzu::Config& config)
{
    bool connected = false;

    while (true) {
        if (!config.debug && testConnection()) {
            if (!connected) {
                logInfo("You have connected to the Internet");
                connected = true;
            }
            std::this_thread::sleep_for(config.check_interval);
            continue;
        }

        connected = false;
        logInfo("Start login in " + std::to_string(config.start_delay.count()) + "s...");
        std::this_thread::sleep_for(config.start_delay);

        try {
            logInfo("Username: " + config.sso_username + ", Service: " + config.campusnet_service);

            yzu::CurlHttpClient client;
            yzu::CampusNet campus_net(client, config.user_agent);

            const auto portal_url = campus_net.getPortalUrl();
            logInfo("Portal url: " + portal_url);

            campus_net.oldLoginService(
                portal_url,
                config.sso_username,
                config.sso_password,
                config.campusnet_service);

            logInfo("Login service success");
        } catch (const std::exception& error) {
            logError(std::string("Failed to login services: ") + error.what());
        }
    }
}

} // namespace

int main(int argc, char* argv[])
{
    try {
        yzu::CurlGlobal curl;
        const auto config = yzu::loadConfig(argc, argv);

        if (config.logout) {
            logout(config);
            return 0;
        }

        loginLoop(config);
        return 0;
    } catch (const std::exception& error) {
        logError(std::string("Fatal: ") + error.what());
        return 1;
    }
}
