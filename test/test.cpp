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
    test_SeparatedGuiAndLogic = 1,
};

namespace {

    /* ----------------------------------- */

    void test_GuiAppAndLogicSep() {
        /* separate thread to start tcp server */
        boost::asio::io_service ios;

        boost::thread_group threads;
        boost::asio::io_context::work work(ios);

        /* separate thread to start tcp server */
        for (uint32_t i = 0; i < boost::thread::hardware_concurrency(); ++i)
        {
            threads.create_thread([&]() {
                work.get_io_context().run();
            });
        }

        std::shared_ptr<User> user = User::CreateNewUser(std::ref(work.get_io_context()));
        boost::asio::post(work.get_io_context(), [&]() {
            user->UserStart();
        });
        boost::asio::post(work.get_io_context(), [&]() {
            Application::InitializeApp(user);
        });

        threads.join_all();
    }

    void tests_start(testcase_t testcase) {

        switch (testcase) {

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

#if SEPARATED_GUI_WITH_LOGIC
    tests_start(Testcase::test_SeparatedGuiAndLogic);
#endif /* SEPARATED_GUI_WITH_LOGIC */
}
