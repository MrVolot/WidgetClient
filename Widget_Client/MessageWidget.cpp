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

void MessageWidget::setText(const QString& text, const QString& time,bool isAuthor)
{
    if(!isAuthor){
        ui->message->setText(text);
        ui->message->setStyleSheet("background-color: #F0C56D;"
                                   "border-top-left-radius :10px;"
                                   "border-top-right-radius : 10px;"
                                   "padding: 2px;");
        ui->time->setStyleSheet("background-color: #F0C56D;"
                                     "border-bottom-left-radius : 100px;"
                                     "border-bottom-right-radius : 10px;"
                                     "padding: 2px;");
        ui->time->setText(time);
        return;
    }
    ui->messageRight->setText(text);
    ui->messageRight->setStyleSheet("background-color: #66ACE1;"
                               "border-top-left-radius :10px;"
                               "border-top-right-radius : 10px;"
                               "padding: 2px;");
    ui->timeRight->setStyleSheet("background-color: #66ACE1;"
                                 "border-bottom-left-radius : 10px;"
                                 "border-bottom-right-radius : 100px;"
                                 "padding: 2px;");
    ui->timeRight->setText(time);
}

QString MessageWidget::getText()
{
    if(ui->message->text().isEmpty()){
        return ui->messageRight->text();
    }else{
        return ui->message->text();
    }
}

MessageWidget::~MessageWidget()
{
    delete ui;
}
