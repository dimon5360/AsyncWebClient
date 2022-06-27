#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <future>

#include <spdlog/spdlog.h>

#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include <boost/asio/ssl.hpp>

#include "core/AppCore.h"

MainWindow::MainWindow(std::shared_ptr<AppCore> core, QWidget *parent, std::string appName_)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      appName(appName_),
      core_(core),
      wmsg(new WidgetMessages(core, this))

{
    ui->setupUi(this);
    this->setWindowTitle(appName_.data());
    ConfigApp();

    appThread = std::thread(&AppCore::AppStart, core_); // TODO: establish
}

MainWindow::~MainWindow()
{
    std::cout << "Destroy MainWindow object\n";
    appThread.join();
    delete ui;
}

void MainWindow::ConfigApp() const noexcept {

    connect(ui->send_msg_btn, &QPushButton::clicked, this, &MainWindow::sign_in_btn_callback);
}

#include <boost/lexical_cast.hpp>

void MainWindow::sign_in_btn_callback() {

    try {
        QString receiverID = this->ui->user_id_tbx->text();
        QString message = this->ui->message_tbx->text();

        auto id = boost::lexical_cast<SecureTcpConnection::user_id_t>(receiverID.toStdString());
        AppCore::GetInstance()->SendMessageToUser(id, std::move(message.toStdString()));

    } catch(std::exception &ex) {
        spdlog::error(ex.what());
    }
}
