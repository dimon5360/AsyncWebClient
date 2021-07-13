/* std C++ lib headers */
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string_view>

/* transform */
#include <algorithm>

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

#include <boost/lexical_cast.hpp>

/* deployment definitions */
#define CONSOLE_LOGGER      1
#define FILE_LOGGER         0

#if FILE_LOGGER
#include <boost/date_time.hpp>
#include <fstream>
#endif /* FILE_LOGGER */

/* Build v.0.0.3 from 14.07.2021 */
const uint32_t PATCH = 3;
const uint32_t MINOR = 0;
const uint32_t MAJOR = 0;

class FileLogger {

private:

    /* open log file */
    int Open() noexcept {
        return 0;
    }

    /* close log file */
    void Close() noexcept {
    }

    /* get time code */
    uint64_t GetCurrTimeMs() {
        const auto systick_now = std::chrono::system_clock::now();
        const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(systick_now.time_since_epoch());
        return nowMs.count();
    }


public:

    FileLogger() {
    }

    ~FileLogger() {
        Close();
    }

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

#include <windows.h>

#include <queue>
#include <mutex>
#include <shared_mutex>

#include <openssl/ssl.h>
#include <boost/asio/ssl.hpp>

class SecureTcpConnection {

    std::queue<uint64_t> msg_queue;
    mutable std::shared_mutex mutex_;

private:

    void to_lower(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

    /***********************************************************************************
     *  @brief  Start process initialization of client
     *  @return None
     */
    void start_init()
    {
        /* hello message to the server */
        std::string msg = "hello server";

        logger.Write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % msg % msg.size()));

        boost::asio::async_write(socket_, boost::asio::buffer(msg),
            boost::bind(&SecureTcpConnection::handle_init, this,
                boost::asio::placeholders::error));
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
                    //start_write();

                    std::thread{ [&]() {

                        while (true) {
                            std::cout << "Which user do you want to send message to?\n";
                            std::string res;
                            std::getline(std::cin, res);

                            uint64_t dstUserId = boost::lexical_cast<uint64_t>(res);
                            std::stringstream msg;

                            msg << tech_msg_header << dstUserId << ", " << tech_resp_msg << randGen->GenRandomNumber();

                            logger.Write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % msg.str() % msg.str().size()));

                            socket_.async_write_some(boost::asio::buffer(msg.str()),
                                [&](const boost::system::error_code& error,
                                    std::size_t bytes_transferred) {
                                        std::cout << "Message sended\n";
                                });

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

            /* support of different register */
            //to_lower(in_msg);

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
#if CHAT
#include <chrono>
#include <future>

#endif /* CHAT */        

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
        std::cout << "Verifying " << subject_name << "\n";

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
        }
    }

    /* unique pointer to random number generator */
    std::unique_ptr<RandomGen> randGen;
    /* cliet nboost tcp socket object */
    //boost::asio::ip::tcp::socket socket_;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;

    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::resolver::results_type endpoints;

    /* msgs headers to exchange with clients */
    const std::string auth_msg = std::string("hello user id=");
    const std::string tech_msg_header = std::string("user id=");
    const std::string tech_resp_msg = std::string("number=");
    const std::string tech_req_msg = std::string("summ=");

    /* unique id of client */
    std::string id_;

    /* exchange data buffer */
    enum { max_length = 1024 };
    boost::array<char, max_length> buf = { { 0 } };

    /* parameters of host and port to connect */
    const std::string host = "localhost";
    const std::string port = "4059";

public:


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


            logger.Write(boost::str(boost::format("Start connecting to %1%:%2% \n") % host % port));
            endpoints = resolver_.resolve(boost::asio::ip::tcp::v4(), host, port);
           
            boost::asio::async_connect(socket_.lowest_layer(), endpoints,
                boost::bind(&SecureTcpConnection::handle_connect, this,
                    boost::asio::placeholders::error));

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

#define SECURE_CONNECTION 1

/* separate thread for processing of ESC key press (to close application) */
static void EscapeWait() {
    while (GetAsyncKeyState(VK_ESCAPE) == 0) {
        Sleep(10);
    }
    exit(0);
}

int main() {
    /* for corrent output boost error messages */
    SetConsoleOutputCP(1251);
    std::cout << boost::format("Hello. Application version is %1%.%2%.%3%\n") % MAJOR % MINOR % PATCH;
    logger.Write("Press ESC to exit...\n");
    try
    {
        /* separate thread to monitor SPACE key pressing */
        std::thread ext(&EscapeWait);
        /* start tcp client */
        boost::asio::io_context io_context;
#if SECURE_CONNECTION
        boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv13);
        ctx.load_verify_file("rootca.crt");
        SecureTcpConnection conn(io_context, ctx);
#endif /* SECURE_CONNECTION */
        io_context.run();
        ext.detach();
    }
    catch (std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
    }

    return 0;
}