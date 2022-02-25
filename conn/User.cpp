/******************************************************
 *  @file   User.cpp
 *
 *  @author Kalmykov Dmitry
 *  @date   16.08.2021
 */

#include "User.h"
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

std::shared_ptr<User> User::user_ = nullptr;

std::string User::SHA256(const std::string& msg)
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

void User::SendMessageToUser(SecureTcpConnection::user_id_t id, std::string&& msg) {

    namespace pt = boost::property_tree;
    pt::ptree ptree;

    auto ownId = conn->GetOwnId();
    ptree.put("user id", id);
    ptree.put("from user id", ownId);
    ptree.put("message", msg);

    std::ostringstream oss;
    boost::property_tree::json_parser::write_json(oss, ptree);

    std::string inifile_text = oss.str();
    std::string json = ptree.data();

    conn->start_write(std::move(inifile_text));
}

#include <boost/coroutine2/all.hpp>

bool User::ProcessAuthResponse() const noexcept {

//    using namespace boost::coroutines;
//    coroutine<void>::pull_type source{[&](){

//                                      }};
    std::future<bool> f = std::async(std::launch::async, [&]{
        while(messageBroker.IsQueueEmpty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        std::string json = messageBroker.PullMessage();
        return dataProcessor->ProcessAuthResponse(std::move(json));
    });
    bool res = f.get();

    std::cout << res << std::endl;
    if(!res) {
        conn->close(boost::system::error_code());
    } else {
        Logger::Debug("User authentication succeed.");
    }
    return res;
}

void User::StartInitialization() {
    conn->start_connect(std::move(GetUserAuthData()));
}

std::string User::GetUserAuthData() const noexcept {
    return dataProcessor->PrepareAuthRequest(boost::str(boost::format("{%1%,%2%}") % username_ % password_));
}

const std::shared_ptr<User>& User::GetInstance() {
    return user_;
}

std::shared_ptr<User> User::CreateUser(boost::asio::io_service& io_service, std::string&& login, std::string&& password) {
    try {
        if(!user_) {
            user_ = std::make_shared<User>(std::move(io_service), std::move(login), std::move(password));
        }
    }
    catch (std::exception& ex) {
        Logger::Error(boost::str(boost::format("CreateNewUser exception: %1%\n") % ex.what()));
    }
    return user_;
}

void User::UserStart() {
    StartInitialization();
    io_service.run();
}

#include <thread>
#include <future>

User::User(boost::asio::io_service&& io_service_, std::string&& login, std::string&& password) :
    io_service(io_service_), ssl_context(boost::asio::ssl::context::tlsv13),
    username_(std::move(login)), password_(std::move(password))
{
    std::cout << "Construct User class\n";
    ssl_context.load_verify_file("rootca.crt");

    conn = std::make_shared<SecureTcpConnection>(io_service_, ssl_context);
    dataProcessor = DataProcessor::CreateProcessor();
}

User::~User() {
    std::cout << "Destruct User class\n";
}
