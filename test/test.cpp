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
#include "../conn/User.h"

/* std C++ lib headers */
#include <memory>

/* external C++ libs headers -------------------------------- */
/* spdlog C++ lib */
#include <spdlog/spdlog.h>

using testcase_t = enum class Testcase {
    test_AppClassInit = 1,
    test_GuiAppWithClient = 2,
    test_SeparatedGuiAndLogic = 3,
};

namespace {

    /* ----------------------------------- */
    void test_AppClassInit() {
        //boost::asio::io_service ios;
        //std::shared_ptr<Application> app = std::make_shared<Application>(std::ref(ios));
        //app->InitializeApp();
    }

    void test_GuiAppWithClientInit() {
        /* separate thread to start tcp server */
        //boost::asio::io_service ios;

        //std::shared_ptr<Application> app = std::make_shared<Application>(std::ref(ios));
        //app->InitializeApp();
    }
    void test_GuiAppAndLogicSep() {
        /* separate thread to start tcp server */
        boost::asio::io_service ios;

        boost::thread_group threads;
        boost::asio::io_context::work work(ios);

        /* separate thread to start tcp server */
        for (int i = 0; i < boost::thread::hardware_concurrency(); ++i)
        {
            threads.create_thread([&]() {
                work.get_io_context().run();
            });
        }

        std::shared_ptr<User> user = User::CreateNewUser(std::ref(work.get_io_context()));
        boost::asio::post(work.get_io_context(), [&]() {
            //Application::InitializeApp(user);
            user->UserStart();
        });
        boost::asio::post(work.get_io_context(), [&]() {
            //user = User::CreateNewUser(std::ref(work.get_io_context()));
            Application::InitializeApp(user);
            //user->UserStart();
        });

        threads.join_all();
        /*boost::asio::post(work.get_io_context(), [&]() {
            Application::InitializeApp(user);
        });*/

        //std::shared_ptr<Application> app = std::make_shared<Application>(std::ref(ios));
        //app->InitializeApp();

        //work.get_io_context().stop();
        //threads.join_all();
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

            case Testcase::test_SeparatedGuiAndLogic:
            {
                test_GuiAppAndLogicSep();
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

#if SEPARATED_GUI_WITH_LOGIC
    tests_start(Testcase::test_SeparatedGuiAndLogic);
#endif /* SEPARATED_GUI_WITH_LOGIC */
}
