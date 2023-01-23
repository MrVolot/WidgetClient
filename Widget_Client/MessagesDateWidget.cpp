#include "MessagesDateWidget.h"
#include "ui_MessagesDateWidget.h"

MessagesDateWidget::MessagesDateWidget(QDateTime dateTime, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessagesDateWidget)
{
    ui->setupUi(this);
    ui->datePlanch->setText(dateTime.toString("MMMM d"));
}

MessagesDateWidget::~MessagesDateWidget()
{
    delete ui;
}
