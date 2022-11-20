#include "MessageWidget.h"
#include "ui_MessageWidget.h"

MessageWidget::MessageWidget(const QString& text, const QString& time, bool isAuthor, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessageWidget)
{
    ui->setupUi(this);
    ui->message->setWordWrap(true);
    ui->messageRight->setWordWrap(true);
    setText(text, time, isAuthor);
}

void MessageWidget::setTextAllignment()
{
    //ui->messageRight->setAlignment(Qt::AlignLeft);
    //ui->msgLayout->setAlignment(Qt::AlignRight);
}

void MessageWidget::setText(const QString& text, const QString& time,bool isAuthor)
{
    if(isAuthor){
        ui->message->setText(text);
        ui->time->setText(time);
        return;
    }
    ui->messageRight->setText(text);
    ui->timeRight->setText(time);
}

MessageWidget::~MessageWidget()
{
    delete ui;
}
