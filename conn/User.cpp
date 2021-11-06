/******************************************************
 *  @file   User.cpp
 *
 *  @author Kalmykov Dmitry
 *  @date   16.08.2021
 */

/* local C++ headers ---------------------------------------- */
#include "User.h"

/* std C++ lib headers -------------------------------------- */
#include <iostream>

/* external C++ libs headers -------------------------------- */
/* cryptopp C++ lib */
#include "cryptopp/hex.h"
#include "cryptopp/base64.h"
#include "cryptopp/sha.h"
#include "cryptopp/hmac.h"
#include "cryptopp/cryptlib.h"
/* boost C++ lib */
#include "boost/regex.hpp"
#include <boost/format.hpp>

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

#include "../utils/json.h"

#include "boost/format.hpp"
#include <boost/property_tree/json_parser.hpp>

void User::SendMessageToUser(SecureTcpConnection::user_id_t id, std::string&& msg) {

    namespace pt = boost::property_tree;
    pt::ptree ptree;

    auto ownId = conn->GetOwnId();
    ptree.put("user id", id);
    ptree.put("from user id", ownId);
    ptree.put("message", msg);

#ifdef DEBUG
    /* traversal the tree through 'foreach' */
    for (auto& item : ptree) {
        auto key = item.first;
        auto value = item.second.data();
        std::cout << key << " : " << value << std::endl;
    }
#endif // DEBUG

    std::ostringstream oss;
    boost::property_tree::json_parser::write_json(oss, ptree);

    std::string inifile_text = oss.str();
    std::string json = ptree.data();

    conn->start_write(std::move(inifile_text));
}

void User::StartInitialization() {
    conn->start_connect(GetUserAuthData());
}

const std::string User::GetUserAuthData() const noexcept {
    return boost::str(boost::format("{%1%,%2%}") % username % password);
}

std::shared_ptr<User> User::CreateNewUser(boost::asio::io_service& io_service) {
    try {
        return std::make_shared<User>(std::move(io_service));
    }
    catch (std::exception& ex) {
        Logger::Write(boost::str(boost::format("CreateNewUser exception: %1%\n") % ex.what()));
    }
}

void User::UserStart() {
    StartInitialization();
    io_service.run();
    std::cout << "User constructor\n";
}

User::User(boost::asio::io_service&& io_service_) :
    io_service(io_service_),
    ssl_context(boost::asio::ssl::context::tlsv13)
{
    ssl_context.load_verify_file("rootca.crt");

    jsonHandler = std::make_unique<JsonHandler>();
    conn = std::make_shared<SecureTcpConnection>(io_service_, ssl_context);
    msgBroker = std::make_unique<MessageBroker>();
}

User::~User() {
    std::cout << "Destruct User class\n";
}