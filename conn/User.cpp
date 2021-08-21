/******************************************************
 *  @file   User.cpp
 *
 *  @author Kalmykov Dmitry
 *  @date   16.08.2021
 */

/* local C++ headers ---------------------------------------- */
#include "User.h"

/* std C++ lib headers -------------------------------------- */
#include <conio.h>

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

bool User::CheckValidPassword(const std::string& pass) {

    boost::regex expression("(?=.*?[a-z])(?=.*[A-Z])(?=.*?[0-9)(?=.*?[#&!@$&*_]).{4,}$");
    boost::smatch what;
    if (boost::regex_match(pass, what, expression))
    {
        return true;
    }
    std::cout << "Incorrect password\n";
    return false;
}

bool User::CheckValidUserName(const std::string& username) {

    return !username.empty();

    boost::regex expression("@(^[a-zA-Z0-9])$"); // TODO: create regexp
    boost::smatch what;
    if (boost::regex_match(username, what, expression))
    {
        return true;
    }
    std::cout << "Incorrect username\n";
    return false;
}

void User::InputUserName() noexcept {
    std::cout << "Enter username: \n";
    std::getline(std::cin, username);
}

void User::InputPassword() noexcept {

    int tryNum = 0;
    do {
        tryNum++;
        std::cout << "Enter password: \n";
        std::string inPass{};
        char ch = 0;
        while (true) {
            ch = _getch();
            if (ch == '\r' || ch == '\r') {
                break;
            }
            else if (ch == 3) {
                return;
            }
            inPass += ch;
        }

        if (CheckValidPassword(inPass)) {
            password = SHA256(inPass);
            if (password.size()) {
                userValid = true;
            }
            return;
        }
        std::cout << boost::str(boost::format("Failed try# %1%: \n") % tryNum);
    } while (!userValid && tryNum < MAX_TRY_NUM);

    if (tryNum > MAX_TRY_NUM) {
        std::cout << "Input password is failed. Too much trying...\n";
    }
}


bool User::StartAuthentication() {
    std::cout << "Authentication ...\n";
    return true;
    /*InputUserName();
    if (!CheckValidUserName(username)) {
        return false;
    }
    InputPassword();
    return userValid;*/
}

/*void User::handle() {
    std::thread{ [&]() {
        while (!msgBroker->IsQueueEmpty()) {
            MessageBroker::message_t msg = msgBroker->PullMessage();

            std::cout << msg.first << " " << msg.second << std::endl;
            //conn->PushMessage(std::move(msg));
            //conn->start_write(std::move(msg));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }}.join();
}*/

void User::SendMessageToUser(const uint64_t dstUsetId, std::string&& msg) {
    //msgBroker->PushMessage(dstUsetId, std::move(msg));
    //conn->PushMessage(std::make_pair(dstUsetId, msg));
    conn->start_write(std::make_pair(dstUsetId, msg));
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
    //if (StartAuthentication()) {
        msgBroker = std::make_unique<MessageBroker>();
        //handle();
        std::cout << "Authentication performed\n";
        StartInitialization();
        io_service.run();
    //}
    std::cout << "User constructor\n";
}

User::User(boost::asio::io_service&& io_service_) :
    io_service(io_service_),
    ssl_context(boost::asio::ssl::context::tlsv13)
{
    ssl_context.load_verify_file("rootca.crt");
    conn = std::make_shared<SecureTcpConnection>(io_service_, ssl_context);
    msgBroker = std::make_unique<MessageBroker>();
    //handle();
    /*std::cout << "Authentication performed\n";
    StartInitialization();
    io_service.run();*/
}

User::~User() {
    std::cout << "Destruct User class\n";
}