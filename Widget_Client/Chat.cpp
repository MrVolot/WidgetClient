#include "Chat.h"
#include "ui_Chat.h"

Chat::Chat(const QString& nameArg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Chat)
{
    ui->setupUi(this);
    ui->receiverName->setText(nameArg);
}

Chat::~Chat()
{
    delete ui;
}

void Chat::on_sendMsgBtn_clicked()
{
    emit sendMessage(ui->msgFiled->text(), 5);
}

