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


void SecureTcpConnection::to_lower(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

void SecureTcpConnection::start_init()
{
    /* hello message to the server */
    std::string msg{ hello_msg_header + info };

    Logger::Write(boost::str(boost::format(">> \"%1%\" [%2%]\n") % msg % msg.size()));

    socket_.async_write_some(boost::asio::buffer(msg),
        [&](const boost::system::error_code& error,
            std::size_t bytes_transferred) {
                if (!error) {
                    start_auth();
                }
                else {
                    Logger::Write(boost::str(boost::format(
                        "Failed initialization: %1%") % error.message()));
                    Shutdown();
                }
        });
}

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

void SecureTcpConnection::start_auth()
{
    socket_.async_read_some(boost::asio::buffer(buf),
        boost::bind(&SecureTcpConnection::handle_auth, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

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
            start_read();
        }
    }
    else {
        Logger::Write(boost::str(boost::format("Failed authentication: %1%") % error.message()));
        Shutdown();
    }
}

void SecureTcpConnection::start_read()
{
    socket_.async_read_some(boost::asio::buffer(buf),
        [&](const boost::system::error_code& error,
            std::size_t bytes_transferred) { 
                if (!error) {
                    std::string_view in_msg{ buf.data(), bytes_transferred };
                    Logger::Write(boost::str(boost::format("<< \"%1%\" [%2%]\n") % in_msg % bytes_transferred));
                    start_read();
                } else {
                    Logger::Write(boost::str(boost::format( "Failed reading: %1%") % error.message()));
                    Shutdown();
                }
        });
}

void SecureTcpConnection::start_write(std::string && msg)
{
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
    std::cout << "Verifying " << subject_name << "\n";
    return preverified;
}

void SecureTcpConnection::start_connect(const std::string& userInfo)
{
    info = userInfo;
    boost::asio::async_connect(socket_.lowest_layer(), endpoints.begin(),
        [&](const boost::system::error_code& error, 
            boost::asio::ip::tcp::resolver::iterator it) {
            if (!error) {
                socket_.async_handshake(boost::asio::ssl::stream_base::client,
                    [&](const boost::system::error_code& error) {
                        if (!error) {
                            start_init();
                        }
                        else {
                            std::cout << "Handshake failed: " << error.message() << "\n";
                            Shutdown();
                        }
                    });
            }
            else {
                std::cout << "Connect failed: " << error.message() << "\n";
                Shutdown();
            }
        });
}


void SecureTcpConnection::Shutdown() {
    std::cout << "Shutdown" << std::endl;
    socket_.async_shutdown([&](const boost::system::error_code& error) {
        std::cout << "Handle of shutdown" << std::endl;
        close(error);
        });
}

/***********************************************************************************
*  @brief  Close connection and call the desctructor
*  @param  error Boost system error object reference
*  @return None
*/
void SecureTcpConnection::close(const boost::system::error_code& error) {
    Logger::Write(boost::str(boost::format("Close connection error: %2% \n") % id_ % error.message()));

    socket_.next_layer().close(); // so is in server side
    //socket_.lowest_layer().close(); // so was in client
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
    }
    catch (std::exception& ex) {
        std::cerr << "Connection failed: " << ex.what() << "\n";
    }
}

SecureTcpConnection::~SecureTcpConnection() {
    std::cout << "SecureTcpConnection destructor.\n";
}