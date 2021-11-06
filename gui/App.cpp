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
#include <nana/gui/widgets/panel.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/scroll.hpp>
/* boost C++ lib headers */
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

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

    try
    {
        using namespace nana;

        //Define a form.
        form fm(nana::rectangle(100, 100, 800, 600));

        static uint16_t fontSize = 16;
        static int8_t fontInc = -2;

        //Define a button and answer the click event.
        button btn{ fm, "Send" };

        textbox messegeTbox{ fm };
        textbox userTbox{ fm };

        label lab{ fm, boost::str(boost::format("Hello, <size=%1%>Nana C++ Library</>") % fontSize) };
        lab.format(true);

        btn.events().click([&] {
            auto id = boost::lexical_cast<uint64_t>(userTbox.text());
            std::string msg = messegeTbox.text();
            std::cout << "message: " << msg << " to user# " << id << std::endl;

            user_->SendMessageToUser(id, std::move(msg));
        });

        //Layout management
        fm.div("vert <><<><weight=80% text><>><><weight=24<><button><>><><weight=24<><weight=30% textbox1><>> \
            <><weight=24<><weight=30% textbox2><>><>");
        fm["button"] << btn;
        fm["text"] << lab;
        fm["textbox1"] << userTbox;
        fm["textbox2"] << messegeTbox;
        fm.collocate();

        //Show the form
        fm.show();

        //Start to event loop process, it blocks until the form is closed.
        exec();
    }
    catch (std::exception& ex)
    {
        spdlog::error(boost::str(boost::format("Exception: %1%\n") % ex.what()));
    }
}


