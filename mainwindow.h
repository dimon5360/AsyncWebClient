#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QDebug>

#include <string>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    std::string appName;
public:
    MainWindow(QWidget *parent = nullptr, std::string appName_ = "DemoApp");
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    void ConfigApp() const noexcept;

private slots:
    void SignInBtnCallback();
};
#endif // MAINWINDOW_H
