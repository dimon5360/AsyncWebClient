/* std C++ lib headers */
#include <iostream>

#include <spdlog/spdlog.h>

/* boost C++ lib headers */
#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/format.hpp>

#include "conn/User.h"

/* Build v.0.0.10 from 06.11.2021 */
const uint32_t PATCH = 10;
const uint32_t MINOR = 0;
const uint32_t MAJOR = 0;

#include "test/test.h"

int main() {

#if UNIT_TEST

    init_unit_tests();
    return 0;
#else 
    /* for corrent output boost error messages */
    SetConsoleOutputCP(1251);
    FileLogger::Write(boost::str(boost::format("Hello. Application version is %1%.%2%.%3%\n") % MAJOR % MINOR % PATCH));

    try
    {
        /* start tcp client */
        boost::asio::io_service ios;
        boost::asio::signal_set signals(ios, SIGINT);
        User::CreateNewUser(std::ref(ios));
        ios.run();
    }
    catch (std::exception &ex)
    {
        spdlog::error(boost::str(boost::format("Exception: %1%\n") % ex.what()));
    }

    return 0;
#endif /* UNIT_TEST */
}