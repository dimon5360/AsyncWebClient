/******************************************************
 *  @file   User.cpp
 *
 *  @author Kalmykov Dmitry
 *  @date   16.08.2021
 */

#include "User.h"

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

void User::StartInitialization() {
    conn->start_connect(std::move(GetUserAuthData()));
}

std::string User::GetUserAuthData() const noexcept {
    return dataProcessor->PrepareAuthData(boost::str(boost::format("{%1%,%2%}") % username_ % password_));
}

std::shared_ptr<User> User::CreateNewUser(boost::asio::io_service& io_service, std::string&& login, std::string&& password) {
    try {
        return std::make_shared<User>(std::move(io_service), std::move(login), std::move(password));
    }
    catch (std::exception& ex) {
        Logger::Write(boost::str(boost::format("CreateNewUser exception: %1%\n") % ex.what()));
    }
    return nullptr;
}

void User::UserStart() {
    StartInitialization();
    io_service.run();
    std::cout << "Construct User class\n";
}

User::User(boost::asio::io_service&& io_service_, std::string&& login, std::string&& password) :
    io_service(io_service_),
    ssl_context(boost::asio::ssl::context::tlsv13),
    username_(std::move(login)),
    password_(std::move(password))
{
    ssl_context.load_verify_file("rootca.crt");

    conn = std::make_shared<SecureTcpConnection>(io_service_, ssl_context);
    dataProcessor = std::make_unique<DataProcessor>();

    std::thread([&](){
        while(true) {
            if(!messageBroker.IsQueueEmpty()) {
                std::string json = messageBroker.PullMessage();
                // TODO: DataProcessor must handle message and to check approving authentication

            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }).detach();
}

User::~User() {
    std::cout << "Destruct User class\n";
}
