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

/* Build v.0.0.5 from 02.08.2021 */
const uint32_t PATCH = 5;
const uint32_t MINOR = 0;
const uint32_t MAJOR = 0;

class FileLogger {

public:

    /* write log string */
    void Write(std::string &&log) noexcept {
        spdlog::info(log);
    }
};

FileLogger logger;

class RandomGen {
private:
    /* random number generator object */
    std::random_device r;
    /* borders of random numbers */
    const int MIN = 0, MAX = 16384;// 8192;//1023; // TODO: max value is enough small

public:

    /* getter for new random number to send in server */
    uint32_t GenRandomNumber() {
        std::default_random_engine e1(r());
        std::uniform_int_distribution<int> uniform_dist(MIN, MAX);

        int value = uniform_dist(e1);
        return static_cast<uint32_t>(value);
    }
};

class SecureTcpConnection {

private:
    friend class SecureSession;

    class SecureSession {

    private:

        std::pair<std::string, uint64_t> session;
        int32_t private_key, common_secret_key;


    public:
        using session_ptr = std::shared_ptr<SecureSession>;

        SecureSession() = delete;

        static session_ptr CreateSession(const std::string& itselfUser, const uint64_t& secondUserId) {
            return std::make_shared<SecureSession>(itselfUser, secondUserId);
        }

        const std::string& PrepareSecureMessage(const std::string& message) {

            return "";
        }

        SecureSession(const std::string& itselfUser, const uint64_t& secondUserId) :
            session({ itselfUser, secondUserId })
        {
            std::cout << "Start secure session\n";

            // Generate first prime numbers
            int32_t p = DH_Crypto::GetRandomPrimeNum(100, 150);
            int32_t g = DH_Crypto::GetRandomPrimeNum(4, 10);

            /* construct random generator */
            std::unique_ptr<DH_Crypto> dh = std::make_unique<DH_Crypto>(p, g);

            private_key = dh->GetRandomPrimeNum(100, 150);
            int32_t A = dh->GetPublicKey();
            int32_t B = dh->Calc(g, private_key, p);
            dh->SetPublicKey(B);
            common_secret_key = dh->Calc(A, private_key, p);
            std::cout << "Client common secret key: " << common_secret_key << std::endl;
        }

        ~SecureSession() {
            std::cout << "Secure session closed\n";
        }
    };


    std::queue<uint64_t> msg_queue;
    mutable std::shared_mutex mutex_;
    std::unordered_map<uint64_t, SecureSession::session_ptr> secSessions;


    void to_lower(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }


    /***********************************************************************************
     *  @brief  Callback-handler of async initialization process
     *  @param  error Boost system error object reference
     *  @return None
     */ 
    void handle_init(const boost::system::error_code& error)
    {
        if (!error) {
            start_auth();
        }
        else {
            logger.Write(boost::str(boost::format(
                "HandleInit error user: %1% \"%2%\"\n") % id_ % error.message()));
            Shutdown();
        }
    }

    /***********************************************************************************
     *  @brief  Start process authentication of client
     *  @return None
     */
    void start_auth()
    {
        socket_.async_read_some(boost::asio::buffer(buf),
            boost::bind(&SecureTcpConnection::handle_auth, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

#define CHAT 1

    void handle_session(const uint64_t dstUserId) {

        if (!secSessions.contains(dstUserId)) {
            /* Create session between two defined users */
            secSessions.emplace(dstUserId, SecureSession::CreateSession(id_, dstUserId));
        }

        std::cout << "What do you want to write?\n";
        std::string messsage;
        std::getline(std::cin, messsage);

        std::string msg{ boost::str(boost::format("%1%%2%, %3%%4%") % tech_msg_header % dstUserId % tech_resp_msg % messsage) };

        logger.Write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % msg % msg.size()));

        socket_.async_write_some(boost::asio::buffer(msg),
            [&](const boost::system::error_code& error,
                std::size_t bytes_transferred) {
                    //std::cout << "Message sended\n";
            });

    }


    /***********************************************************************************
     *  @brief  Callback-handler of async authentication process
     *  @param  error Boost system error object reference
     *  @param  recvBytes Amount of bytes received from connection
     *  @return None
     */
    void handle_auth(const boost::system::error_code& error,
        std::size_t bytes_transferred)
    {
        if (!error) {

            std::string in_auth_msg{ buf.data(), bytes_transferred };
            logger.Write(boost::str(boost::format("<< \"%1%\" [%2%]\n") % in_auth_msg % bytes_transferred));

            /* support of different register */
            to_lower(in_auth_msg);

            /* check that auth msg corresponds to default value */
            if (in_auth_msg.substr(0, auth_msg.size()).compare(auth_msg) == 0) {
                id_ = in_auth_msg.substr(auth_msg.size());

#if CHAT
                try {

                    start_read();

                    std::thread{ [&]() {
                        while (true) {
                            std::cout << "Which user do you want to send message to?\n";
                            std::string res;
                            std::getline(std::cin, res);

                            uint64_t dstUserId = boost::lexical_cast<uint64_t>(res);

                            //std::thread{ [&]() {
                                handle_session(dstUserId);
                            //} }.join();


                           /* secSessions.emplace(dstUserId, SecureSession::CreateSession(id_, dstUserId));

                            std::stringstream msg;

                            msg << tech_msg_header << dstUserId << ", " << tech_resp_msg << randGen->GenRandomNumber();

                            logger.Write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % msg.str() % msg.str().size()));

                            socket_.async_write_some(boost::asio::buffer(msg.str()),
                                [&](const boost::system::error_code& error,
                                    std::size_t bytes_transferred) {
                                        //std::cout << "Message sended\n";
                                });*/

                            std::this_thread::sleep_for(std::chrono::milliseconds(4));
                        }
                    } }.detach();
                }
                catch (std::exception& ex) {
                    std::cout << ex.what();
                }
#else 
                start_read();
#endif /* CHAT */
            }
        }
        else {
            Shutdown();
        }
    }

    /***********************************************************************************
     *  @brief  Start async reading process from socket
     *  @param  None
     *  @return None
     */
    void start_read()
    {
        socket_.async_read_some(boost::asio::buffer(buf),
            boost::bind(&SecureTcpConnection::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    /***********************************************************************************
     *  @brief  Callback-handler of async reading process
     *  @param  error Boost system error object reference
     *  @param  recvBytes Amount of bytes received from connection
     *  @return None
     */
    void handle_read(const boost::system::error_code& error,
        std::size_t bytes_transferred)
    {
        if (!error) {

            std::string_view in_msg{ buf.data(), bytes_transferred };

            logger.Write(boost::str(boost::format("<< \"%1%\" [%2%]\n") % in_msg % bytes_transferred));

#if CHAT
            start_read();
#else 
            if (in_msg.starts_with(tech_msg_header)) {
                auto item = in_msg.find(tech_req_msg);
                uint64_t value = boost::lexical_cast<uint64_t>(in_msg.substr(item + tech_req_msg.size()));
                start_write();
            }
#endif /* CHAT */
        }
        else {
            Shutdown();
        }
    }  

    /***********************************************************************************
     *  @brief  Start async writing process from socket
     *  @param  None
     *  @return None
     */
    void start_write()
    {
#if CHAT
        if (!msg_queue.empty()) {

            std::stringstream msg;
            uint64_t dstUserId = msg_queue.front();

            msg << tech_msg_header << id_ << ", " << tech_resp_msg << randGen->GenRandomNumber();

            logger.Write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % msg.str() % msg.str().size()));

            boost::asio::async_write(socket_, boost::asio::buffer(msg.str()),
                boost::bind(&SecureTcpConnection::handle_write, this,
                    boost::asio::placeholders::error));
        }
#else 
        std::stringstream msg;
        msg << tech_msg_header << id_ << ", " << tech_resp_msg << randGen->GenRandomNumber();

        logger.Write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % msg.str() % msg.str().size()));

        boost::asio::async_write(socket_, boost::asio::buffer(msg.str()),
            boost::bind(&SecureTcpConnection::handle_write, this,
                boost::asio::placeholders::error));
#endif /* CHAT */        
    }

    /***********************************************************************************
     *  @brief  Callback-handler of async writing process
     *  @param  error Boost system error object reference
     *  @return None
     */
    void handle_write(const boost::system::error_code& error)
    {
        if (!error) {
#if CHAT
            start_write();
#else 
            start_read();
#endif /* CHAT */
        }
        else {
            Shutdown();
        }
    }

    bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx)
    {
        // The verify callback can be used to check whether the certificate that is
        // being presented is valid for the peer. For example, RFC 2818 describes
        // the steps involved in doing this for HTTPS. Consult the OpenSSL
        // documentation for more details. Note that the callback is called once
        // for each certificate in the certificate chain, starting from the root
        // certificate authority.

        // In this example we will simply print the certificate's subject name.
        char subject_name[256];
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
        //std::cout << "Verifying " << subject_name << "\n";

        return preverified;
    }

    void handle_connect(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_.async_handshake(boost::asio::ssl::stream_base::client,
                boost::bind(&SecureTcpConnection::handle_handshake, this,
                    boost::asio::placeholders::error));
        }
        else
        {
            std::cout << "Connect failed: " << error.message() << "\n";
            Shutdown();
        }
    }
    void handle_handshake(const boost::system::error_code& error)
    {
        if (!error)
        {
            start_init();
        }
        else
        {
            std::cout << "Handshake failed: " << error.message() << "\n";
            Shutdown();
        }
    }

    /* unique pointer to random number generator */
    std::unique_ptr<RandomGen> randGen;
    /* alias for ssl stream to tcp socket */
    using ssl_socket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
    /* cliet boost ssl socket object */
    ssl_socket socket_;

    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::resolver::results_type endpoints;

    /* msgs headers to exchange with clients */
    const std::string auth_msg{ "hello user id=" };
    const std::string hello_msg_header{ "hello server" };
    const std::string tech_msg_header{ "user id=" };
    const std::string tech_resp_msg{ "message=" };
    const std::string tech_req_msg{ "summ=" };

    /* unique id of client */
    std::string id_;

    /* exchange data buffer */
    enum { max_length = 1024 };
    boost::array<char, max_length> buf = { { 0 } };

    /* parameters of host and port to connect */
    const std::string host = "localhost";
    const std::string port = "4059";
    std::string info;

    /***********************************************************************************
     *  @brief  Start process initialization of client
     *  @return None
     */
    void start_init()
    {
        /* hello message to the server */
        std::string msg{ hello_msg_header + info };

        logger.Write(boost::str(boost::format(">> \"%1%,%2%\" [%2%]\n") % msg % msg.size()));

        boost::asio::async_write(socket_, boost::asio::buffer(msg),
            boost::bind(&SecureTcpConnection::handle_init, this,
                boost::asio::placeholders::error));
    }

public:

    void start_connect(const std::string& userInfo)
    {
        info = userInfo;
        boost::asio::async_connect(socket_.lowest_layer(), endpoints,
            boost::bind(&SecureTcpConnection::handle_connect, this,
                boost::asio::placeholders::error));
    }


    void Shutdown() {
        std::cout << "Shutdown" << std::endl;
        socket_.async_shutdown(boost::bind(&SecureTcpConnection::HandleShutdown, this, boost::asio::placeholders::error));
        //socket_.next_layer().close();
    }

    void HandleShutdown(const boost::system::error_code& error) {
        std::cout << "Handle of shutdown" << std::endl;
        //socket_.shutdown();
        close(error);
    }


    /***********************************************************************************
     *  @brief  Close connection and call the desctructor
     *  @param  error Boost system error object reference
     *  @return None
     */
    void close(const boost::system::error_code& error) {
        logger.Write(boost::str(boost::format("Close connection error: %2% \n") % id_ % error.message()));
        
        socket_.lowest_layer().close();
    }

    SecureTcpConnection(boost::asio::io_context& io_context, boost::asio::ssl::context &ssl_context)
        : socket_(io_context, ssl_context),
        resolver_(io_context) 
    {
        std::cout << "SecureTcpConnection constructor.\n";
        try {
            socket_.set_verify_mode(boost::asio::ssl::verify_peer);
            socket_.set_verify_callback(boost::bind(&SecureTcpConnection::verify_certificate, this, 
                boost::placeholders::_1, boost::placeholders::_2));
            //socket_.set_verify_callback(boost::asio::ssl::rfc2818_verification("google.com"/*boost::str(boost::format("%1%:%2%") % host % port)*/));


            logger.Write(boost::str(boost::format("Start connecting to %1%:%2% \n") % host % port));
            endpoints = resolver_.resolve(boost::asio::ip::tcp::v4(), host, port);

            /*boost::asio::async_connect(socket_.lowest_layer(), endpoints,
                boost::bind(&SecureTcpConnection::handle_connect, this,
                    boost::asio::placeholders::error));*/

            randGen = std::make_unique<RandomGen>();
        }
        catch (std::exception& ex) {
            std::cerr << "Connection failed: " << ex.what() << "\n";
        }

    }

    ~SecureTcpConnection() {
        std::cout << "SecureTcpConnection destructor.\n";
    }
};

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
                if (ch == '\r') {
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
            logger.Write(boost::str(boost::format("CreateNewUser exception: %1%\n") % ex.what()));
        }
    }
};

int main() {
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
}