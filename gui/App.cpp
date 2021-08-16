/******************************************************
 *  @file   App.cpp
 *  @brief  Application GUI class
 * 
 *  @author Kalmykov Dmitry
 *  @date   11.08.2021
 */

/* local C++ headers */
#include "App.h"

/* std C++ lib headers */
#include "iostream"

/* external C++ GUI lib headers - nana */
#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>

/* boost C++ lib headers */
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

Application::Application() {
    std::cout << "Construct Application class\n";
}

Application::~Application() {
    std::cout << "Destroy Application class\n";
}


void Application::InitializeApp() {

    using namespace nana;

    //Define a form.
    form fm;

    static uint16_t fontSize = 16;
    static int8_t fontInc = -2;

    //Define a button and answer the click event.
    //button btn(fm, nana::rectangle(20, 20, 140, 40));
    //btn.set_bground("Decrease font size");
    button btn{ fm, "Decrease font size" };
    //btn.enable_focus_color(true);

    label lab{ fm, boost::str(boost::format("Hello, <bold blue size=%1%>Nana C++ Library</>") % fontSize) };
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
        lab.caption(boost::str(boost::format("Hello, <bold blue size=%1%>Nana C++ Library</>") % fontSize));
    });

    //Layout management
    fm.div("vert <><<><weight=80% text><>><><weight=24<><button><>><>");
    fm["button"] << btn;
    fm["text"] << lab;
    fm.collocate();

    //Show the form
    fm.show();

    //Start to event loop process, it blocks until the form is closed.
    exec();
}


