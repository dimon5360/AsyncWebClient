/******************************************************
 *  @file   User.cpp
 *
 *  @author Kalmykov Dmitry
 *  @date   16.08.2021
 */

#include "AppCore.h"
#include "../log/logger.h"

#include <iostream>

#include "cryptopp/hex.h"
#include "cryptopp/base64.h"
#include "cryptopp/sha.h"
#include "cryptopp/hmac.h"
#include "cryptopp/cryptlib.h"

#include "boost/regex.hpp"
#include <boost/format.hpp>
#include <boost/property_tree/json_parser.hpp>

MessageBroker messageBroker;

std::shared_ptr<AppCore> AppCore::user_ = nullptr;

void AppCore::SetUserId(const uint32_t id_) {
    conn_->SetId(id_);
}

std::string AppCore::SHA256(const std::string& msg)
{
    using namespace CryptoPP;
    CryptoPP::SHA256 hash;

    const size_t ss{ msg.size() };
    byte const* pbData = (byte*)msg.data();
    byte abDigest[CryptoPP::SHA256::DIGESTSIZE];

    hash.CalculateDigest(abDigest, pbData, ss);

    //HexEncoder encoder;
    Base64Encoder encoder;
    std::string output;
    encoder.Attach(new CryptoPP::StringSink(output));
    encoder.Put(abDigest, sizeof(abDigest));
    encoder.MessageEnd();

    return output;
}

#include "../format/json.h"

#include "boost/format.hpp"

void AppCore::SendMessageToUser(SecureTcpConnection::user_id_t id, std::string&& msg) {
    conn_->start_write(std::move(dataProcessor->PrepareUserMessage(std::move(msg), conn_->GetOwnId(), id)));
}

#include <boost/coroutine2/all.hpp>

bool AppCore::ProcessAuthResponse() const noexcept {

    bool res = false;
    try {
        std::future<bool> f = std::async(std::launch::async, [&]{
            while(messageBroker.IsQueueEmpty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            std::string json = messageBroker.PullMessage();
            return dataProcessor->ProcessAuthResponse(std::move(json));
        });

        res = f.get();

        if(!res) {
            conn_->close(boost::system::error_code());
        } else {
            Logger::Debug("User authentication succeed.");
        }
    } catch (std::exception &ex) {
        spdlog::error(boost::str(boost::format("%1 %2%") % "ProcessAuthResponse error: " % ex.what()));
    }

    return res;
}

void AppCore::StartInitialization() {
    conn_->start_connect(std::move(GetUserAuthData()));
}

std::string AppCore::GetUserAuthData() const noexcept {
    return dataProcessor->PrepareAuthRequest(boost::str(boost::format("{%1%,%2%}") % username_ % password_));
}

const std::shared_ptr<AppCore>& AppCore::GetInstance() {
    return user_;
}

std::shared_ptr<AppCore> AppCore::CreateApp(boost::asio::io_service& io_service, std::string&& login, std::string&& password) {

    user_ = std::make_shared<AppCore>(std::ref(io_service), std::move(login), std::move(password));
    return user_;
}

void AppCore::AppStart() {
    StartInitialization();
    io_service.run();
}

#include <thread>
#include <future>

AppCore::AppCore(boost::asio::io_service& io_service_, std::string&& login, std::string&& password) :
    io_service(io_service_), ssl_context(boost::asio::ssl::context::tlsv13),
    username_(std::move(login)), password_(std::move(password))
{
    std::cout << "Construct User class\n";
    ssl_context.load_verify_file("rootca.crt");

    conn_ = std::make_shared<SecureTcpConnection>(io_service_, ssl_context);
    dataProcessor = DataProcessor::CreateProcessor();
}

AppCore::~AppCore() {
    std::cout << "Destruct User class\n";
}
