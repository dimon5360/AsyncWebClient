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

    std::unique_ptr<MessageBroker> msgBroker;

    std::string username_{}, password_{};
    bool userValid = false;

    const int MAX_TRY_NUM = 3;

    static std::string SHA256(const std::string& msg);

    std::shared_ptr<SecureTcpConnection> conn;
    boost::asio::ssl::context ssl_context;
    boost::asio::io_service& io_service;
    std::shared_ptr<DataProcessor> dataProcessor;

    static std::shared_ptr<User> user_;

    // remove copy constructor and copy assignment operator
    User(const User& copiedUser) = delete;
    User& operator=(const User& copiedUser) = delete;
    // to avoid creating another one instances
    User();

    void StartInitialization();

public:

    void SetUserId(const SecureTcpConnection::user_id_t& id_) {
        conn->SetId(id_);
    }

    std::string GetUserAuthData() const noexcept;
    void SendMessageToUser(SecureTcpConnection::user_id_t dstUsetId, std::string&& msg);

    bool ProcessAuthResponse() const noexcept;

    void UserStart();

    static const std::shared_ptr<User>& GetInstance();
    static std::shared_ptr<User> CreateUser(boost::asio::io_service& io_service,
                                            std::string&& login, std::string&& password_);
    User(boost::asio::io_service&& io_service, std::string&& login, std::string&& password);
    ~User();
};
