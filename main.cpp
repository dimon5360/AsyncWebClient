#include "mainwindow.h"

#include <QApplication>

#include <spdlog/spdlog.h>
#include <boost/format.hpp>
#include <memory>

/* Build v.0.0.11 from 28.02.2022 */
const uint32_t PATCH = 11;
const uint32_t MINOR = 0;
const uint32_t MAJOR = 0;

int main(int argc, char *argv[])
{
    std::string appHeader = boost::str(boost::format("SecureWebChat v.%1%.%2%.%3%") % MAJOR % MINOR % PATCH);
    spdlog::info(boost::str(boost::format("Hello. Application %1%\n") % appHeader));

    QApplication a(argc, argv);

    auto w = std::make_shared<MainWindow>(nullptr, appHeader);
    w->show();
    return a.exec();
}
