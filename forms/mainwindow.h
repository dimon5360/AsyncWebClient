#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QStringList>

#include <string>

#include "core/AppCore.h"
#include "forms/widgetmessages.h"


QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(std::shared_ptr<AppCore> core_, QWidget *parent = nullptr, std::string appName_ = "DemoApp");
    ~MainWindow();

private:

    std::thread appThread;
    std::string appName;
    std::shared_ptr<AppCore> core_;

    Ui::MainWindow *ui;
    WidgetMessages *wmsg;

    void ConfigApp() const noexcept;

private slots:
    void sign_in_btn_callback();
};

class LoginDialog : public QDialog
{
    Q_OBJECT
private:
    QLabel *labelUsername;
    QLabel *labelPassword;
    QComboBox *comboUsername;
    QLineEdit *editPassword;
    QDialogButtonBox *buttons;

    QString login_{}, password_{};

    void setUpGUI();

public:
    explicit LoginDialog(QWidget *parent);

    void setUsername(const QString &username);
    void setPassword(const QString &password);

    void setUsernamesList(const QStringList &usernames);
    std::pair<QString, QString> GetAuth() {
        return {login_, password_};
    }

signals:

    void acceptLogin(QString &username, QString &password, int &indexNumber);

private slots:

    void slotAcceptLogin();
};
#endif // MAINWINDOW_H
