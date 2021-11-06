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

    std::shared_ptr<SecureTcpConnection> conn;
    boost::asio::ssl::context ssl_context;
    boost::asio::io_service& io_service;
    std::unique_ptr<JsonHandler> jsonHandler;

    void StartInitialization();

public:
    void UserStart();
    void SendMessageToUser(SecureTcpConnection::user_id_t dstUsetId, std::string&& msg);
    const std::string GetUserAuthData() const noexcept;
    static std::shared_ptr<User> CreateNewUser(boost::asio::io_service& io_service);

    User(boost::asio::io_service&& io_service);
    ~User();

};