#include "MessageWidget.h"
#include "ui_MessageWidget.h"

MessageWidget::MessageWidget(const QString& text, const QString& time, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessageWidget)
{
    ui->setupUi(this);
    ui->time->setText(time);
    ui->message->setWordWrap(true);
    ui->message->setText(text);
}

MessageWidget::~MessageWidget()
{
    delete ui;
}
