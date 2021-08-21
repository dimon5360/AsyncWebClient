/******************************************************
 *  @file   App.h
 *  @brief  Application GUI class declaration
 *
 *  @author Kalmykov Dmitry
 *  @date   11.08.2021
 */
#pragma once

 /* local C++ headers */
#include "../conn/User.h"

/* external C++ libs headers -------------------------------- */
/* boost C++ lib headers */
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/asio.hpp>

/* GUI C++ lib - nana */
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>

class Application {

    //std::shared_ptr<User> user;
    //boost::thread_group threads;
    //boost::asio::io_service& io_service;
    //boost::asio::io_context::work work;

public:

    //Application(boost::asio::io_service& ios);
    //Application(std::shared_ptr<User> user);
    //~Application();
    
    static void InitializeApp(std::shared_ptr<User> user);
};