#ifndef WIDGETMESSAGES_H
#define WIDGETMESSAGES_H

#include "ui_widgetmessages.h"
#include <QWidget>

#include "core/AppCore.h"

// namespace Ui {
//     class WidgetMessages;
// }

class WidgetMessages : QWidget
{
    Q_OBJECT
public:
    explicit WidgetMessages(std::shared_ptr<AppCore>& core, QWidget *parent = nullptr);
    virtual ~WidgetMessages();

private:

    Ui::WidgetMessages *ui;
    std::shared_ptr<AppCore> core_;
};

#endif // WIDGETMESSAGES_H
