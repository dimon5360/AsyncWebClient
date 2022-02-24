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
#include "../data/DataProcess.h"

/* std C++ lib headers */
#include <shared_mutex>
#include <queue>

class User {

    friend class SecureTcpConnection;

private:
    /* to keep msg between App GUI and TCP connection handler */
    std::unique_ptr<MessageBroker> msgBroker;

    std::string username_{}, password_{};
    bool userValid = false;

    const int MAX_TRY_NUM = 3;

    static std::string SHA256(const std::string& msg);

    std::shared_ptr<SecureTcpConnection> conn;
    boost::asio::ssl::context ssl_context;
    boost::asio::io_service& io_service;
    std::unique_ptr<DataProcessor> dataProcessor;

    void StartInitialization();

public:
    void UserStart();
    void SendMessageToUser(SecureTcpConnection::user_id_t dstUsetId, std::string&& msg);
    std::string GetUserAuthData() const noexcept;
    static std::shared_ptr<User> CreateNewUser(boost::asio::io_service& io_service, std::string&& login, std::string&& password_);

    User(boost::asio::io_service&& io_service, std::string&& login, std::string&& password);
    ~User();
};
