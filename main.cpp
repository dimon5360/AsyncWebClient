#include "forms/mainwindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QString>

#include <spdlog/spdlog.h>
#include <boost/format.hpp>
#include <memory>

/* Build v.0.0.12 from 27.06.2022 */
const uint32_t PATCH = 12;
const uint32_t MINOR = 0;
const uint32_t MAJOR = 0;

LoginDialog::LoginDialog(QWidget *parent)
{
    setUpGUI();
    setWindowTitle(tr("User Login"));
    setModal(true);
}

void LoginDialog::setUpGUI()
{
    // set up the layout
    QGridLayout *formGridLayout = new QGridLayout(this);

    // initialize the username combo box so that it is editable

    comboUsername = new QComboBox(this);
    comboUsername->setEditable(true);
    // initialize the password field so that it does not echo
    // characters
    editPassword = new QLineEdit(this);
    editPassword->setEchoMode(QLineEdit::Password);

    // initialize the labels

    labelUsername = new QLabel(this);
    labelPassword = new QLabel(this);
    labelUsername->setText(tr("Username"));
    labelUsername->setBuddy(comboUsername);
    labelPassword->setText(tr("Password"));
    labelPassword->setBuddy(editPassword);

    // initialize buttons

    buttons = new QDialogButtonBox(this);
    buttons->addButton(QDialogButtonBox::Ok);
    buttons->addButton(QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Login"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Abort"));

    // connects slots
    connect(buttons->button(QDialogButtonBox::Cancel),
            SIGNAL(clicked()),
            this,
            SLOT(close()));

    connect(buttons->button(QDialogButtonBox::Ok),
            SIGNAL(clicked()),
            this,
            SLOT(slotAcceptLogin()));

    formGridLayout->addWidget(labelUsername, 0, 0);
    formGridLayout->addWidget(comboUsername, 0, 1);
    formGridLayout->addWidget(labelPassword, 1, 0);
    formGridLayout->addWidget(editPassword, 1, 1);
    formGridLayout->addWidget(buttons, 2, 0, 1, 2);

    setLayout(formGridLayout);
}

void LoginDialog::setUsername(const QString &username)
{

    bool found = false;
    for (int i = 0; i < comboUsername->count() && !found; i++)
    {
        if (comboUsername->itemText(i) == username)
        {
            comboUsername->setCurrentIndex(i);
            found = true;
        }
    }

    if (!found)
    {

        int index = comboUsername->count();
        qDebug() << "Select username " << index;
        comboUsername->addItem(username);

        comboUsername->setCurrentIndex(index);
    }
    editPassword->setFocus();

}

void LoginDialog::setPassword(const QString &password)
{
    editPassword->setText(password);
}

#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include <boost/asio/ssl.hpp>

#include "core/AppCore.h"

boost::asio::io_service ios;
        
void LoginDialog::slotAcceptLogin()
{
    QString username = comboUsername->currentText();
    QString password = editPassword->text();
    int index = comboUsername->currentIndex();
    
    login_ = username;
    password_ = password;
    
    spdlog::info("Login: " + login_.toStdString());
    spdlog::info("Password: " + password_.toStdString());

    emit acceptLogin(username, // current username
                     password, // current password
                     index     // index in the username list
    );
    close();
}

void LoginDialog::setUsernamesList(const QStringList &usernames)
{
    comboUsername->addItems(usernames);
}

int main(int argc, char *argv[])
{
    std::string appHeader = boost::str(boost::format("SecureWebChat v.%1%.%2%.%3%") % MAJOR % MINOR % PATCH);
    spdlog::info(boost::str(boost::format("Hello. Application %1%\n") % appHeader));

    QApplication a(argc, argv);

    LoginDialog *loginDialog = new LoginDialog(nullptr);
    loginDialog->setUsername("Test");
    loginDialog->setUsername("Admin");
    loginDialog->setPassword("pass1");
    loginDialog->setPassword("pass2");

    loginDialog->exec();

    auto [login, password] = loginDialog->GetAuth();

    auto app = AppCore::CreateApp(std::ref(ios), std::move(login.toStdString()),
            std::move(password.toStdString()));

    auto w = std::make_shared<MainWindow>(app, nullptr, appHeader);

    w->show();
    spdlog::info("Application start");
    
    auto res = a.exec();
    return res;
}
