/******************************************************
 *  @file   SecureAsyncConnection.h
 *
 *  @author Kalmykov Dmitry
 *  @date   16.08.2021
 */

#pragma once

#include "MessageBroker.h"
#include "../format/json.h"

#include <string>
#include <memory>
#include <queue>
#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <set>

#include <spdlog/spdlog.h>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/array.hpp>

class SecureTcpConnection {

    friend class User;
public:

    using user_id_t = uint32_t;
    user_id_t GetOwnId() const noexcept {
        return id_;
    }

private:

    std::set<user_id_t> sessions;

    void to_lower(std::string& str);

    void start_init();
    void handle_init(const boost::system::error_code& error);
    void start_auth();
    void handle_auth(const boost::system::error_code& error,
        std::size_t bytes_transferred);
    void start_read();
    bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx);


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

    user_id_t id_;

    enum { max_length = 1024 };
    boost::array<char, max_length> buf = { { 0 } };

    const std::string host = "localhost";
    const std::string port = "4059";
    std::string jsonAuth;
    mutable std::shared_mutex mutex_;

public:

    void start_write(std::string&& msg);
    void start_connect(std::string&& userInfo);
    void Shutdown();
    void close(const boost::system::error_code& error);

    SecureTcpConnection(boost::asio::io_context& io_context, boost::asio::ssl::context& ssl_context);
    ~SecureTcpConnection();

protected:

    std::condition_variable cv;
    void SetId(const user_id_t& id) {
        id_ = id;
    }
};
