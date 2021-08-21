/******************************************************
 *  @file   SecureAsyncConnection.cpp
 *
 *  @author Kalmykov Dmitry
 *  @date   16.08.2021
 */


/* local C++ headers */
#include "SecureAsyncConnection.h"

/* std C++ lib headers */
#include <cstdlib>
#include <cstring>
#include <iostream>

/* transform */

/* random */
#include <iomanip>
#include <map>
#include <cmath>


#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include "boost/regex.hpp"

#include "cryptopp/hex.h"
#include "cryptopp/base64.h"
#include "cryptopp/sha.h"
#include "cryptopp/hmac.h"
#include "cryptopp/cryptlib.h"


decltype(auto) SecureTcpConnection::SecureSession::CreateSession(const uint64_t& itselfUser, const uint64_t& secondUserId) {
    return std::make_shared<SecureSession>(itselfUser, secondUserId);
}

const int32_t SecureTcpConnection::SecureSession::GetPublickKey() {
    return public_key;
}

const std::string& SecureTcpConnection::SecureSession::PrepareSecureMessage(const std::string& message) {

    return "";
}

bool SecureTcpConnection::SecureSession::IsSessionSecure() {
    return bIsSessionSecure;
}

void SecureTcpConnection::SecureSession::CalcCommonSecret(const int32_t& A) {
    dh->GenCommonSecret(A);
    bIsSessionSecure = true;
}

SecureTcpConnection::SecureSession::SecureSession(const uint64_t& itselfUser, const uint64_t& secondUserId) :
    session({ itselfUser, secondUserId })
{
    std::cout << "Start secure session\n";

    /* construct random generator */
    dh = std::make_unique<DH_Crypto>();
    public_key = dh->GetPublicKey();
}

SecureTcpConnection::SecureSession::~SecureSession() {
    std::cout << "Secure session closed\n";
}



void SecureTcpConnection::to_lower(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}


/***********************************************************************************
*  @brief  Callback-handler of async initialization process
*  @param  error Boost system error object reference
*  @return None
*/
void SecureTcpConnection::handle_init(const boost::system::error_code& error)
{
    if (!error) {
        start_auth();
    }
    else {
        Logger::Write(boost::str(boost::format(
            "HandleInit error user: %1% \"%2%\"\n") % id_ % error.message()));
        Shutdown();
    }
}

/***********************************************************************************
*  @brief  Start process authentication of client
*  @return None
*/
void SecureTcpConnection::start_auth()
{
    socket_.async_read_some(boost::asio::buffer(buf),
        boost::bind(&SecureTcpConnection::handle_auth, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

#define CHAT 1

const int32_t SecureTcpConnection::establish_session(const uint64_t& dstUserId) {
    /* Create session between two defined users */
    secSessions.emplace(dstUserId, SecureSession::CreateSession(id_, dstUserId));
    return secSessions[dstUserId]->GetPublickKey();
}

void SecureTcpConnection::finish_establish_session() {
    socket_.async_read_some(boost::asio::buffer(buf),
        boost::bind(&SecureTcpConnection::handle_establish_secure, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void SecureTcpConnection::handle_establish_secure(const boost::system::error_code& error,
    std::size_t bytes_transferred) {

    std::cout << __func__ << std::endl;

    if (!error) {
        try {

            std::string in_msg{ buf.data(), bytes_transferred };
            Logger::Write(boost::str(boost::format("<< \"%1%\" [%2%]\n") % in_msg % bytes_transferred));

            if (in_msg.substr(0, tech_msg_header.size()).compare(tech_msg_header) == 0) {

                size_t ss = in_msg.find(",") - tech_msg_header.size();
                std::string s = in_msg.substr(tech_msg_header.size(), ss);
                auto dstUserId = boost::lexical_cast<uint64_t>(s.substr(0, s.find(":")));
                auto srcUserId = boost::lexical_cast<uint64_t>(s.substr(s.find(":") + 1, s.size() - s.find(":")));

                if (!secSessions.contains(dstUserId)) {
                    /* if session is not started yet */
                    std::cout << "Session with user #" << dstUserId << " is not started yet\n";
                    int32_t pubKey = establish_session(dstUserId);
                    auto key = in_msg.substr(in_msg.find(tech_pub_key_msg) + tech_pub_key_msg.size(), in_msg.size());
                    std::cout << "In public key " << key << std::endl;
                    std::cout << "Out public key " << pubKey << std::endl;
                    secSessions[dstUserId]->CalcCommonSecret(boost::lexical_cast<int32_t>(key));
                    SendPublicKey(srcUserId, dstUserId, pubKey);
                }
                else {
                    /* if session is already started */
                    std::cout << "Session with user #" << dstUserId << " is already started\n";
                    auto key = in_msg.substr(in_msg.find(tech_pub_key_msg) + tech_pub_key_msg.size(), in_msg.size());
                    secSessions[dstUserId]->CalcCommonSecret(boost::lexical_cast<int32_t>(key));
                }
            }
            else {
                std::cout << "Tech message header is incorrect\n";
            }
        }
        catch (std::exception& ex) {
            std::cout << ex.what() << std::endl;
            return;
        }

        finish_establish_session();
    }
    else {
        Shutdown();
    }
}

void SecureTcpConnection::SendPublicKey(const uint64_t& srcUserId, const uint64_t& dstUserId, const int32_t& key) {

    auto req = boost::str(boost::format("%1%%2%:%3%, %4%%5%")
        % tech_msg_header % srcUserId % dstUserId % tech_pub_key_msg % key);

    Logger::Write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % req % req.size()));

    socket_.async_write_some(boost::asio::buffer(req),
        [&](const boost::system::error_code& error,
            std::size_t bytes_transferred) {
                std::cout << "Public key sended\n";
        });
}

void SecureTcpConnection::handle_session(const uint64_t& dstUserId) {

    if (dstUserId == id_) {
        return;
    }

    if (!secSessions.contains(dstUserId)) {
        int32_t pubKey = establish_session(dstUserId);
        SendPublicKey(id_, dstUserId, pubKey);
    }
    else if (secSessions[dstUserId]->IsSessionSecure()) {

        std::cout << "What do you want to write?\n";
        std::string messsage;
        std::getline(std::cin, messsage);

        std::cout << messsage << std::endl;

        std::string msg = boost::str(boost::format("%1%%2%:%3%, %3%%4%")
            % tech_msg_header % id_ % dstUserId % tech_resp_msg % messsage);

        Logger::Write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % msg % msg.size()));

        socket_.async_write_some(boost::asio::buffer(msg),
            [&](const boost::system::error_code& error,
                std::size_t bytes_transferred) {
            });
    }
}


/***********************************************************************************
*  @brief  Callback-handler of async authentication process
*  @param  error Boost system error object reference
*  @param  recvBytes Amount of bytes received from connection
*  @return None
*/
void SecureTcpConnection::handle_auth(const boost::system::error_code& error,
    std::size_t bytes_transferred)
{
    if (!error) {

        std::string in_auth_msg{ buf.data(), bytes_transferred };
        Logger::Write(boost::str(boost::format("<< \"%1%\" [%2%]\n") % in_auth_msg % bytes_transferred));

        /* support of different register */
        to_lower(in_auth_msg);

        /* check that auth msg corresponds to default value */
        if (in_auth_msg.substr(0, auth_msg.size()).compare(auth_msg) == 0) {
            id_ = boost::lexical_cast<uint64_t>(in_auth_msg.substr(auth_msg.size()));
#if CHAT
#if 0
            try {

                finish_establish_session();

                std::thread{ [&]() {
                    while (true) {
                        std::cout << "Which user do you want to send message to?\n";
                        std::string res;
                        std::getline(std::cin, res);

                        uint64_t dstUserId;
                        try {
                            dstUserId = boost::lexical_cast<uint64_t>(res);
                        }
                        catch (std::exception& ex) {
                            std::cout << ex.what() << std::endl;
                            return;
                        }
                        handle_session(dstUserId);
                        std::this_thread::sleep_for(std::chrono::milliseconds(4));
                    }
                } }.detach();
            }
            catch (std::exception& ex) {
                std::cout << ex.what();
            }
            #else 
#endif 
            start_read();
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
void SecureTcpConnection::start_read()
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
void SecureTcpConnection::handle_read(const boost::system::error_code& error,
    std::size_t bytes_transferred)
{
    if (!error) {

        std::string_view in_msg{ buf.data(), bytes_transferred };

        Logger::Write(boost::str(boost::format("<< \"%1%\" [%2%]\n") % in_msg % bytes_transferred));

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
void SecureTcpConnection::start_write(MessageBroker::message_t&& msgData)
{
#if CHAT
    //if (!msg_queue.empty()) {

        //std::stringstream msg;
        //MessageBroker::message_t msgData = msg_queue.front();

        //msg << tech_msg_header << id_ << ", " << tech_resp_msg << randGen->GenRandomNumber();

        std::string msg{ boost::str(boost::format("%1%%2%, %3%%4%") % tech_msg_header % msgData.first % tech_resp_msg % msgData.second) };
        Logger::Write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % msg % msg.size()));

        socket_.async_write_some(boost::asio::buffer(msg),
            [&](const boost::system::error_code& error,
                std::size_t bytes_transferred) {
                if (error) {
                    Logger::Write(boost::str(boost::format(
                        "Invalid hello message from user %1%: \"%2%\"\n") % id_ % error.message()));
                    Shutdown();
                }
            });

            /*boost::bind(&SecureTcpConnection::handle_write, this,
                boost::asio::placeholders::error));*/
    //}
#else 
    std::stringstream msg;
    msg << tech_msg_header << id_ << ", " << tech_resp_msg << randGen->GenRandomNumber();

    FileLogger::Write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % msg.str() % msg.str().size()));

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
void SecureTcpConnection::handle_write(const boost::system::error_code& error)
{
    if (!error) {
#if CHAT
        //start_write();
#else 
        start_read();
#endif /* CHAT */
    }
    else {
        Shutdown();
    }
}

bool SecureTcpConnection::verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx)
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

void SecureTcpConnection::handle_connect(const boost::system::error_code& error)
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
void SecureTcpConnection::handle_handshake(const boost::system::error_code& error)
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

/***********************************************************************************
*  @brief  Start process initialization of client
*  @return None
*/
void SecureTcpConnection::start_init()
{
    /* hello message to the server */
    std::string msg{ hello_msg_header + info };

    Logger::Write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % msg % msg.size()));

    boost::asio::async_write(socket_, boost::asio::buffer(msg),
        boost::bind(&SecureTcpConnection::handle_init, this,
            boost::asio::placeholders::error));
}

void SecureTcpConnection::start_connect(const std::string& userInfo)
{
    info = userInfo;
    boost::asio::async_connect(socket_.lowest_layer(), endpoints,
        boost::bind(&SecureTcpConnection::handle_connect, this,
            boost::asio::placeholders::error));
}


void SecureTcpConnection::Shutdown() {
    std::cout << "Shutdown" << std::endl;
    socket_.async_shutdown(boost::bind(&SecureTcpConnection::HandleShutdown, this, boost::asio::placeholders::error));
    //socket_.next_layer().close();
}

void SecureTcpConnection::HandleShutdown(const boost::system::error_code& error) {
    std::cout << "Handle of shutdown" << std::endl;
    //socket_.shutdown();
    close(error);
}


/***********************************************************************************
*  @brief  Close connection and call the desctructor
*  @param  error Boost system error object reference
*  @return None
*/
void SecureTcpConnection::close(const boost::system::error_code& error) {
    Logger::Write(boost::str(boost::format("Close connection error: %2% \n") % id_ % error.message()));

    socket_.lowest_layer().close();
}

SecureTcpConnection::SecureTcpConnection(boost::asio::io_context& io_context, boost::asio::ssl::context& ssl_context)
    : socket_(io_context, ssl_context),
    resolver_(io_context)
{
    std::cout << "SecureTcpConnection constructor.\n";
    try {
        socket_.set_verify_mode(boost::asio::ssl::verify_peer);
        socket_.set_verify_callback(boost::bind(&SecureTcpConnection::verify_certificate, this,
            boost::placeholders::_1, boost::placeholders::_2));

        Logger::Write(boost::str(boost::format("Start connecting to %1%:%2% \n") % host % port));
        endpoints = resolver_.resolve(boost::asio::ip::tcp::v4(), host, port);

        randGen = std::make_unique<RandomGen>();
    }
    catch (std::exception& ex) {
        std::cerr << "Connection failed: " << ex.what() << "\n";
    }
}

SecureTcpConnection::~SecureTcpConnection() {
    std::cout << "SecureTcpConnection destructor.\n";
}