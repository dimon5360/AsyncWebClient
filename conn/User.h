/******************************************************
 *  @file   User.h
 *
 *  @author Kalmykov Dmitry
 *  @date   16.08.2021
 */

#pragma once

/* local C++ headers */
#include "SecureAsyncConnection.h"
#include "MessageBroker.h"

/* std C++ lib headers */
#include <shared_mutex>
#include <queue>

class User {
private:
    /* to keep msg between App GUI and TCP connection handler */
    std::unique_ptr<MessageBroker> msgBroker;

    std::string username{}, password{};
    bool userValid = false;

    const int MAX_TRY_NUM = 3;

    static std::string SHA256(const std::string& msg);

    static bool CheckValidPassword(const std::string& pass);

    static bool CheckValidUserName(const std::string& username);

    void InputUserName() noexcept;

    void InputPassword() noexcept;

    std::shared_ptr<SecureTcpConnection> conn;
    boost::asio::ssl::context ssl_context;
    boost::asio::io_service& io_service;
    void handle();
    bool StartAuthentication();

    void StartInitialization();
public:
    void UserStart();

    void SendMessageToUser(const uint64_t dstUsetId, std::string&& msg);

    User(boost::asio::io_service&& io_service);
    ~User();

    const std::string GetUserAuthData() const noexcept;

    static std::shared_ptr<User> CreateNewUser(boost::asio::io_service& io_service);
};