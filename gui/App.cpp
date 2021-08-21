/******************************************************
 *  @file   App.cpp
 *  @brief  Application GUI class implementation
 * 
 *  @author Kalmykov Dmitry
 *  @date   11.08.2021
 */

 /* local C++ headers ---------------------------------------- */
#include "App.h"
#include "../conn/User.h"

/* std C++ lib headers -------------------------------------- */
#include "iostream"

/* external C++ libs headers -------------------------------- */
/* GUI C++ lib - nana */
#include <nana/gui.hpp>
/* boost C++ lib headers */
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

/*Application::Application(boost::asio::io_service& ios) :
    io_service(std::ref(ios)),
    work(ios)
{
    std::cout << "Construct Application class\n";

    for (int i = 0; i < boost::thread::hardware_concurrency(); ++i)
    {
        threads.create_thread([&]() {
            work.get_io_context().run();
        });
    }
}*/

/*Application::Application(std::shared_ptr<User> user_):
    user(user_)
{
    std::cout << "Construct Application class\n";
}*/

/*Application::~Application() {
    std::cout << "Destroy Application class\n";
    //work.get_io_context().stop();
    //threads.join_all();
}*/

// TODO:
/******************************************************************************************
 * Realized:
 *      1. Button "Send" to get message from textbox and print to console
 *      2. Textbox to input message 
 *      3. Label with common information
 * 
 * Need to realize:
 *      1. Big text box with scroll to out received message from another users
 *      2. Struct with clickable fields to select user whom you want to send message
 *      3. Send entered message from textbox through TCP channel to another user
 *      4. Out entered message into the big textbox
 *      5. Out received into big textbox (for each specific user)
 *      6. Open form with big textbox for each user by clicking on its field in struct
 ******************************************************************************************/

void Application::InitializeApp(std::shared_ptr<User> user_) {

    //std::shared_ptr<User> user;

    try
    {
        //std::thread{ [&]() {
            using namespace nana;

            //Define a form.
            form fm;

            static uint16_t fontSize = 16;
            static int8_t fontInc = -2;

            //Define a button and answer the click event.
            //button btn(fm, nana::rectangle(20, 20, 140, 40));
            button btn{ fm, "Send" };

            textbox tbox{ fm/*, nana::rectangle(20, 20, 140, 40)*/ };
            //tbox.borderless(true);

            label lab{ fm, boost::str(boost::format("Hello, <size=%1%>Nana C++ Library</>") % fontSize) };
            lab.format(true);

            std::thread tUserConn;

            btn.events().click([&] {
                std::string tx = tbox.text();
                std::cout << tx << std::endl;
                user_->SendMessageToUser(10, std::move(tx));
            });

            //Layout management
            fm.div("vert <><<><weight=80% text><>><><weight=24<><button><>><><weight=24<><weight=60% textbox><>><>");
            //fm.div("vert <weight=80% text><weight=24 button><weight=100% textbox>");
            fm["button"] << btn;
            fm["text"] << lab;
            fm["textbox"] << tbox;
            fm.collocate();

            //Show the form
            fm.show();

            //Start to event loop process, it blocks until the form is closed.
            exec();
        //} }.join();
    }
    catch (std::exception& ex)
    {
        spdlog::error(boost::str(boost::format("Exception: %1%\n") % ex.what()));
    }
}


