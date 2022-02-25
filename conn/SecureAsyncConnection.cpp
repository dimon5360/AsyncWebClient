/******************************************************
 *  @file   SecureAsyncConnection.cpp
 *
 *  @author Kalmykov Dmitry
 *  @date   16.08.2021
 */

#include "SecureAsyncConnection.h"
#include "MessageBroker.h"
#include "User.h"

#include "../format/json.h"
#include "../log/logger.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <map>
#include <cmath>


#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include "boost/regex.hpp"
#include <boost/property_tree/json_parser.hpp>

void SecureTcpConnection::to_lower(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

// @brief Start exchange with server, send auth message
void SecureTcpConnection::start_init()
{
    std::string msg{ jsonAuth };
    Logger::Debug(boost::str(boost::format(">> \"%1%\" [%2%]\n") % msg % msg.size()));

    socket_.async_write_some(boost::asio::buffer(msg),
        [&](const boost::system::error_code& error,
            std::size_t bytes_transferred) {
                if (!error) {
                    start_auth();
                } else {
                    Logger::Error(boost::str(boost::format(
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
        Logger::Error(boost::str(boost::format(
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
        Logger::Debug(boost::str(boost::format("<< \"%1%\" [%2%]\n") % in_auth_msg % bytes_transferred));

        to_lower(in_auth_msg);

        messageBroker.PushMessage(std::move(in_auth_msg));
//        User::GetInstance()->ProcessAuthResponse();
//        if(User::GetInstance()->ProcessAuthResponse()) {
            start_read();
//        }
    }
    else {
        Logger::Error(boost::str(boost::format("Failed authentication: %1%") % error.message()));
        Shutdown();
    }
}

void SecureTcpConnection::start_read()
{
    socket_.async_read_some(boost::asio::buffer(buf),
        [&](const boost::system::error_code& error,
            std::size_t bytes_transferred) { 
                if (!error) {
                    std::string in_msg{ buf.data(), bytes_transferred };
                    Logger::Debug(boost::str(boost::format("<< \"%1%\" [%2%]\n") % in_msg % bytes_transferred));
                    start_read();
                } else {
                    Logger::Error(boost::str(boost::format( "Failed reading: %1%") % error.message()));
                    Shutdown();
                }
        });
}

void SecureTcpConnection::start_write(std::string && msg)
{
    Logger::Debug(boost::str(boost::format(">> \"%1%\" [%2%]\n") % msg % msg.size()));

    socket_.async_write_some(boost::asio::buffer(msg),
        [&](const boost::system::error_code& error,
            std::size_t bytes_transferred) {
                if (error) {
                    Logger::Error(boost::str(boost::format(
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

void SecureTcpConnection::start_connect(std::string&& authConnectionjson)
{
    jsonAuth = authConnectionjson;
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

void SecureTcpConnection::close(const boost::system::error_code& error) {
    Logger::Error(boost::str(boost::format("Close connection error: %2% \n") % id_ % error.message()));

    socket_.next_layer().close(); // so is in server side
    //socket_.lowest_layer().close(); // so was in client
}

SecureTcpConnection::SecureTcpConnection(boost::asio::io_context& io_context, boost::asio::ssl::context& ssl_context)
    : socket_(io_context, ssl_context),
    resolver_(io_context)
{
    std::cout << "Construct SecureTcpConnection class.\n";
    try {
        socket_.set_verify_mode(boost::asio::ssl::verify_peer);
        socket_.set_verify_callback(boost::bind(&SecureTcpConnection::verify_certificate, this,
            boost::placeholders::_1, boost::placeholders::_2));

        Logger::Debug(boost::str(boost::format("Start connecting to %1%:%2% \n") % host % port));
        endpoints = resolver_.resolve(boost::asio::ip::tcp::v4(), host, port);
    }
    catch (std::exception& ex) {
        std::cerr << "Connection failed: " << ex.what() << "\n";
    }
}

SecureTcpConnection::~SecureTcpConnection() {
    Logger::Debug("Destruct SecureTcpConnection class.");
}
