#include "widgetmessages.h"

WidgetMessages::WidgetMessages(std::shared_ptr<AppCore> &core, QWidget *parent):
    QWidget(parent), core_(core), ui(new Ui::WidgetMessages)
{
    ui->setupUi(this);
}

WidgetMessages::~WidgetMessages() {
    delete ui;
}
