/******************************************************
 *  @file   User.h
 *
 *  @author Kalmykov Dmitry
 *  @date   16.08.2021
 */

#pragma once

#include "../data/MessageBroker.h"
#include "../data/DataProcess.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>


#include <shared_mutex>
#include <queue>
#include <memory>

#include "SecureAsyncConnection.h"
class AppCore {

    friend class SecureTcpConnection;

private:

    std::unique_ptr<MessageBroker> msgBroker;

    std::string username_{}, password_{};
    bool userValid = false;

    const int MAX_TRY_NUM = 3;

    static std::string SHA256(const std::string& msg);

    std::shared_ptr<SecureTcpConnection> conn_;
    boost::asio::ssl::context ssl_context;
    boost::asio::io_service& io_service;
    std::shared_ptr<DataProcessor> dataProcessor;

    static std::shared_ptr<AppCore> user_;

    // remove copy constructor and copy assignment operator
    AppCore(const AppCore& copiedUser) = delete;
    AppCore& operator=(const AppCore& copiedUser) = delete;
    // to avoid creating another one instances
    AppCore() = delete;

    void StartInitialization();

public:

    void SetUserId(const uint32_t id_);

    std::string GetUserAuthData() const noexcept;
    void SendMessageToUser(uint32_t dstUsetId, std::string&& msg);

    bool ProcessAuthResponse() const noexcept;

    void AppStart();

    static const std::shared_ptr<AppCore>& GetInstance();
    static std::shared_ptr<AppCore> CreateApp(boost::asio::io_service& io_service,
                                            std::string&& login, std::string&& password_);
    AppCore(boost::asio::io_service& io_service, std::string&& login, std::string&& password);
    ~AppCore();
};
