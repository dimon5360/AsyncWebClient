/******************************************************
 *  @file   SecureAsyncConnection.h
 *
 *  @author Kalmykov Dmitry
 *  @date   16.08.2021
 */

#pragma once

/* local C++ headers ---------------------------------------- */
#include "../crypto/dh.h"

/* std C++ lib headers -------------------------------------- */
#include <string>
#include <memory>
#include <queue>
#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <random>

/* external C++ libs headers -------------------------------- */
/* spdlog C++ lib */
#include <spdlog/spdlog.h>
/* boost C++ lib */
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/array.hpp>

class FileLogger {

public:

    /* write log string */
    static void Write(std::string&& log) noexcept {
        spdlog::info(log);
    }
};

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

        std::pair<uint64_t, uint64_t> session;
        int32_t public_key;

        //int32_t p, g;
        std::unique_ptr<DH_Crypto> dh;
        bool bIsSessionSecure = false;

    public:
        using session_ptr = std::shared_ptr<SecureSession>;

        SecureSession() = delete;

        static decltype(auto) CreateSession(const uint64_t& itselfUser, const uint64_t& secondUserId);

        const int32_t GetPublickKey();

        const std::string& PrepareSecureMessage(const std::string& message);

        bool IsSessionSecure();

        void CalcCommonSecret(const int32_t& A);

        SecureSession(const uint64_t& itselfUser, const uint64_t& secondUserId);
        ~SecureSession();
    };


    std::queue<uint64_t> msg_queue;
    mutable std::shared_mutex mutex_;
    std::unordered_map<uint64_t, SecureSession::session_ptr> secSessions;


    void to_lower(std::string& str);

    /***********************************************************************************
     *  @brief  Callback-handler of async initialization process
     *  @param  error Boost system error object reference
     *  @return None
     */
    void handle_init(const boost::system::error_code& error);

    /***********************************************************************************
     *  @brief  Start process authentication of client
     *  @return None
     */
    void start_auth();

#define CHAT 1

    const int32_t establish_session(const uint64_t& dstUserId);
    void finish_establish_session();

    void handle_establish_secure(const boost::system::error_code& error,
        std::size_t bytes_transferred);

    void SendPublicKey(const uint64_t& srcUserId, const uint64_t& dstUserId, const int32_t& key);

    void handle_session(const uint64_t& dstUserId);


    /***********************************************************************************
     *  @brief  Callback-handler of async authentication process
     *  @param  error Boost system error object reference
     *  @param  recvBytes Amount of bytes received from connection
     *  @return None
     */
    void handle_auth(const boost::system::error_code& error,
        std::size_t bytes_transferred);

    /***********************************************************************************
     *  @brief  Start async reading process from socket
     *  @param  None
     *  @return None
     */
    void start_read();

    /***********************************************************************************
     *  @brief  Callback-handler of async reading process
     *  @param  error Boost system error object reference
     *  @param  recvBytes Amount of bytes received from connection
     *  @return None
     */
    void handle_read(const boost::system::error_code& error,
        std::size_t bytes_transferred);

    /***********************************************************************************
     *  @brief  Start async writing process from socket
     *  @param  None
     *  @return None
     */
    void start_write();

    /***********************************************************************************
     *  @brief  Callback-handler of async writing process
     *  @param  error Boost system error object reference
     *  @return None
     */
    void handle_write(const boost::system::error_code& error);

    bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx);

    void handle_connect(const boost::system::error_code& error);
    void handle_handshake(const boost::system::error_code& error);

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
    const std::string tech_pub_key_msg{ "key=" };

    /* unique id of client */
    uint64_t id_;

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
    void start_init();

public:

    void start_connect(const std::string& userInfo);


    void Shutdown();

    void HandleShutdown(const boost::system::error_code& error);


    /***********************************************************************************
     *  @brief  Close connection and call the desctructor
     *  @param  error Boost system error object reference
     *  @return None
     */
    void close(const boost::system::error_code& error);

    SecureTcpConnection(boost::asio::io_context& io_context, boost::asio::ssl::context& ssl_context);

    ~SecureTcpConnection();
};