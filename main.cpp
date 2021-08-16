/* std C++ lib headers */
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string_view>

/* transform */
#include <algorithm>

#include <queue>
#include <mutex>
#include <shared_mutex>

/* random */
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>

#include <spdlog/spdlog.h>

/* boost C++ lib headers */
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/format.hpp>

#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <boost/asio/ssl.hpp>

#include <boost/lexical_cast.hpp>

#include "cryptopp/hex.h"
#include "cryptopp/base64.h"
#include "cryptopp/sha.h"
#include "cryptopp/hmac.h"
#include "cryptopp/cryptlib.h"


#include "conio.h"

#include "boost/regex.hpp"

#include "crypto/dh.h"
#include "conn/SecureAsyncConnectoin.h"

/* Build v.0.0.6 from 16.08.2021 */
const uint32_t PATCH = 5;
const uint32_t MINOR = 0;
const uint32_t MAJOR = 0;

class User {
private:
    std::string username{}, password{};
    bool userValid = false;

    const int MAX_TRY_NUM = 3;

    static std::string SHA256(const std::string& msg)
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

    static bool CheckValidPassword(const std::string& pass) {

        boost::regex expression("(?=.*?[a-z])(?=.*[A-Z])(?=.*?[0-9)(?=.*?[#&!@$&*_]).{4,}$");
        boost::smatch what;
        if (boost::regex_match(pass, what, expression))
        {
            return true;
        }
        std::cout << "Incorrect password\n";
        return false;
    }

    static bool CheckValidUserName(const std::string& username) {

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

    void InputUserName() noexcept {
        std::cout << "Enter username: \n";
        std::getline(std::cin, username);
    }

    void InputPassword() noexcept {

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

    std::shared_ptr<SecureTcpConnection> conn;
    boost::asio::ssl::context ssl_context;
public:

    bool StartAuthentication() {
        std::cout << "Authentication ...\n";
        return true;
        InputUserName();
        if (!CheckValidUserName(username)) {
            return false;
        }
        InputPassword();
        return userValid;
    }

    void StartInitialization() {
        conn->start_connect(GetUserAuthData());
    }

    User(boost::asio::io_service&& io_service) :
        ssl_context(boost::asio::ssl::context::tlsv13)
    {
        ssl_context.load_verify_file("rootca.crt");
        conn = std::make_shared<SecureTcpConnection>(io_service, ssl_context);
        if (StartAuthentication()) {
            std::cout << "Authentication performed\n";
            StartInitialization();
            io_service.run();
        }
    }

    ~User() {
        std::cout << "Destruct User class\n";
    }

    const std::string GetUserAuthData() const noexcept {
        return boost::str(boost::format("{%1%,%2%}") % username % password);
    }

    static void CreateNewUser(boost::asio::io_service& io_service) {
        try {
            std::make_unique<User>(std::move(io_service));
        }
        catch (std::exception& ex) {
            FileLogger::Write(boost::str(boost::format("CreateNewUser exception: %1%\n") % ex.what()));
        }
    }
};

#include "test/test.h"

int main() {

#ifdef UNIT_TEST

    init_unit_tests();
    return 0;
#else 
    /* for corrent output boost error messages */
    SetConsoleOutputCP(1251);
    std::cout << boost::format("Hello. Application version is %1%.%2%.%3%\n") % MAJOR % MINOR % PATCH;
    try
    {
        /* start tcp client */
        boost::asio::io_service ios;
        boost::asio::signal_set signals(ios, SIGINT);
        User::CreateNewUser(std::ref(ios));
    
        /* asynchronous wait for Ctrl + C signal to occur */ // TODO: signal does not work
        /*signals.async_wait([&](const boost::system::error_code& error, int signal_number) {
            ios.stop();
            throw "Signal is Ctrl + C\n";
        });*/
        ios.run();
    }
    catch (std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
    }

    return 0;
#endif /* UNIT_TEST */
}