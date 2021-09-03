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

class panel_scrolled : public nana::panel<false>
{
public:

    /** CTOR

        @param[in] parent window
        @param[in] visible_panel size of visible window, including scroll bars
        @param[in] scrolled_panel size of area over which window moves

    The window parent/child arrangement:

    <pre>

    parent
        <----- myPanelAndScrolls
                          <----------Visible
                                    <----------------BASE ( scrolled panel )
                          <----------Vertical Scrol
                          <--------- Horizontal Scroll
    </pre>

    */
    panel_scrolled(
        const nana::window& parent,
        const nana::rectangle& visible_panel,
        const nana::rectangle& scrolled_panel)
        : myVisibleAndScrolls(parent, true),
        myVisible(myVisibleAndScrolls, true),
        myScrollHoriz(myVisibleAndScrolls, true),
        myScrollVert(myVisibleAndScrolls, true),
        myPlace(myVisibleAndScrolls)
    {
        // create base panel over which scrollable window moves
        create(myVisible, true);
        move(scrolled_panel);

        // place scrolling window and scrolls in position on parent
        myVisibleAndScrolls.move(visible_panel);

        // Set scroll ammounts to the full extent of the scrolled area
        myScrollHoriz.amount(scrolled_panel.width);
        myScrollVert.amount(scrolled_panel.height);

        // register scroll event handlers
        myScrollHoriz.events().value_changed([&]
            {
                move(-(int)myScrollHoriz.value(), -(int)myScrollVert.value());
            });
        myScrollVert.events().value_changed([&]
            {
                move(-(int)myScrollHoriz.value(), -(int)myScrollVert.value());
            });

        // arrange visible panel with scrolls at right and bottom
        myPlace.div("<vert <<panel><scroll_vert weight=16>> <scroll_horiz weight=16>>");
        myPlace["panel"] << myVisible;
        myPlace["scroll_vert"] << myScrollVert;
        myPlace["scroll_horiz"] << myScrollHoriz;
        myPlace.collocate();

    }
private:
    nana::panel<false> myVisibleAndScrolls;   /// Window showing scrolling window and scrolls
    nana::panel<false> myVisible;           ///< Part of scrolled panel that is visible
    nana::scroll< false > myScrollHoriz;
    nana::scroll< true  > myScrollVert;
    nana::place myPlace;
};


void Application::InitializeApp(std::shared_ptr<User> user_) {

    try
    {
        using namespace nana;

        //Define a form.
        form fm(nana::rectangle(100, 100, 800, 600));

        /* construct scrolling window
        //panel_scrolled S(
        //    fm,
        //    { 50, 50, 200, 200 },        // size and location of scrolling window on form
        //    { 0, 0, 500, 500 });

        // add a big label to be scrolled over
        nana::label lbl(S, nana::rectangle(
            0, 0,
            500, 300));
        lbl.caption(
            "This is a very very very very very wide label\n"
            " This is a very very very very very wide label with 2nd line\n"
            " This is a very very very very very wide label and 3rd line\n"
            " This is a very very very very very wide label and 4th line\n"
            " This is a very very very very very wide label and 5th line\n"
            " This is a very very very very very wide label and 6th line\n"
            " This is a very very very very very wide label and 7th line\n"
            " This is a very very very very very wide label and 8th line\n"
            " This is a very very very very very wide label and 9th line");


        // show & run
        fm.show();
        nana::exec();
        return;*/

        static uint16_t fontSize = 16;
        static int8_t fontInc = -2;

        //Define a button and answer the click event.
        //button btn(fm, nana::rectangle(20, 20, 140, 40));
        button btn{ fm, "Send" };

        textbox tbox{ fm/*, nana::rectangle(20, 20, 140, 40)*/ };

        label lab{ fm, boost::str(boost::format("Hello, <size=%1%>Nana C++ Library</>") % fontSize) };
        lab.format(true);

        btn.events().click([&] {
            std::string tx = tbox.text();
            std::cout << tx << std::endl;
            user_->SendMessageToUser(10, std::move(tx));
        });

        //Layout management
        fm.div("vert <><<><weight=80% text><>><><weight=24<><button><>><><weight=24<><weight=60% textbox><>><>");
        fm["button"] << btn;
        fm["text"] << lab;
        fm["textbox"] << tbox;
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


