/******************************************************
 *  @file   test.cpp
 *  @brief  Unit tests implementations
 *
 *  @author Kalmykov Dmitry
 *  @date   11.08.2021
 */

 /* local C++ headers */
#include "test.h"
#include "../gui/App.h"

/* std C++ lib headers */
#include <memory>

/* external C++ libs headers -------------------------------- */
/* spdlog C++ lib */
#include <spdlog/spdlog.h>

using testcase_t = enum class Testcase {
    test_AppClassInit = 1,
    test_GuiAppWithClient = 2,
};

namespace {

    /* ----------------------------------- */
    void test_AppClassInit() {
        boost::asio::io_service ios;
        std::shared_ptr<Application> app = std::make_shared<Application>(std::ref(ios));
        app->InitializeApp();
    }

    void test_GuiAppWithClientInit() {
        /* separate thread to start tcp server */
        boost::asio::io_service ios;
        std::shared_ptr<Application> app = std::make_shared<Application>(std::ref(ios));
        app->InitializeApp();
    }

    void tests_start(testcase_t testcase) {

        switch (testcase) {

            case Testcase::test_AppClassInit:
            {
                test_AppClassInit();
                break;
            }

            case Testcase::test_GuiAppWithClient:
            {
                test_GuiAppWithClientInit();
                break;
            }

            default: 
            {
                spdlog::error("Undefined test case");
            }
        }
    }
}

void init_unit_tests() {
#if GUI_APP_INIT
    tests_start(Testcase::test_AppClassInit);
#endif /* GUI_APP_INIT */

#if GUI_APP_WITH_CLIENT_INIT
    tests_start(Testcase::test_GuiAppWithClient);
#endif /* GUI_APP_WITH_CLIENT_INIT */
}
