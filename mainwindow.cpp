#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <spdlog/spdlog.h>

#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include <boost/asio/ssl.hpp>

#include "conn/User.h"

#define DEBUG_AUTH

MainWindow::MainWindow(QWidget *parent, std::string appName_)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      appName(appName_)
{
    ui->setupUi(this);
    this->setWindowTitle(appName_.data());
    ConfigApp();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ConfigApp() const noexcept {

    QPushButton *btn =  this->ui->sign_in_btn;
    connect(btn, SIGNAL(clicked()), this, SLOT(SignInBtnCallback()));
}

void MainWindow::SignInBtnCallback() {
    qDebug() << "Sign In button clicked";

    QString login, password;
    login = this->ui->login_tbx->text();
    password = this->ui->password_tbx->text();

#ifdef DEBUG_AUTH
    qDebug() << "Login textbox contains the next text: " << login;
    qDebug() << "Password textbox contains the next text: " << password;
#endif // DEBUG_AUTH

    std::thread([&](){
        try
        {
            boost::asio::io_service ios;
            auto user = User::CreateUser(std::ref(ios), std::move(login.toStdString()), std::move(password.toStdString()));
            user->UserStart();
            ios.run();
        }
        catch (std::exception &ex)
        {
            qDebug() << "Exception: " << ex.what();
        }
     }).detach();

    // TODO: exit from handler, redraw the window, process tcp connection
}
