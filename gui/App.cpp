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

Application::Application(boost::asio::io_service& ios) :
    io_service(std::ref(ios)),
    work(ios)
{
    std::cout << "Construct Application class\n";

    /* separate thread to start tcp server */
    for (int i = 0; i < boost::thread::hardware_concurrency(); ++i)
    {
        threads.create_thread([&]() {
            work.get_io_context().run();
        });
    }
}

Application::~Application() {
    std::cout << "Destroy Application class\n";
    work.get_io_context().stop();
    threads.join_all();
}


void Application::InitializeApp() {

    try
    {
        boost::asio::post(work.get_io_context(), [&]() {
            User::CreateNewUser(std::ref(io_service));
        });

        using namespace nana;

        //Define a form.
        form fm;

        static uint16_t fontSize = 16;
        static int8_t fontInc = -2;

        //Define a button and answer the click event.
        button btn{ fm, "Decrease font size" };

        textbox txt_{ fm };
        txt_.borderless(true);

        label lab{ fm, boost::str(boost::format("Hello, <size=%1%>Nana C++ Library</>") % fontSize) };
        lab.format(true);

        btn.events().click([&] {

            if (fontInc < 0 && fontSize < 12) {
                std::cout << "Increase font size\n";
                fontInc = 2;
                btn.caption("Increase font size");
            } else if (fontInc > 0 && fontSize > 18) {
                std::cout << "Decrease font size\n";
                fontInc = -2;
                btn.caption("Decrease font size");
            }

            fontSize += fontInc;
            lab.caption(boost::str(boost::format("Hello, <size=%1%>Nana C++ Library</>") % fontSize));
        });

        //Layout management
        fm.div("vert <weight=80% text><weight=24 button><textbox>");
        fm["button"] << btn;
        fm["text"] << lab;
        fm["textbox"] << txt_;
        fm.collocate();

        //Show the form
        fm.show();

        //Start to event loop process, it blocks until the form is closed.
        exec();
        //ios.run();
    }
    catch (std::exception& ex)
    {
        spdlog::error(boost::str(boost::format("Exception: %1%\n") % ex.what()));
    }
}


