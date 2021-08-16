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


using testcase_t = enum class Testcase {
    test_AppClassInit = 1,
};

namespace AppClassTests {

    /* ----------------------------------- */
    void test_AppClassInit() {
        std::unique_ptr<Application> app = std::make_unique<Application>();
        app->InitializeApp();
    }
}

namespace {

    void tests_start(testcase_t testcase) {

        switch (testcase) {

        case Testcase::test_AppClassInit:
        {
            AppClassTests::test_AppClassInit();
            break;
        }

        }
    }
}

void init_unit_tests() {
    tests_start(Testcase::test_AppClassInit);
}
