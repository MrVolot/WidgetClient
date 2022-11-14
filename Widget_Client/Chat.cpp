#include "Chat.h"
#include "ui_Chat.h"
#include <sstream>
//#include <ctime>
#include <iomanip>

Chat::Chat(const QString& nameArg, unsigned long long idArg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Chat),
    name{nameArg},
    id{idArg}
{
    ui->setupUi(this);
    ui->receiverName->setText(name);
}

Chat::~Chat()
{
    delete ui;
}

void Chat::on_sendMsgBtn_clicked()
{
    QString meesageToSend{ui->msgFiled->text()};
    receiveMessage(meesageToSend, 0);
    ui->msgFiled->clear();
    emit sendMessage(meesageToSend, id);
}

void Chat::receiveMessage(const QString &msg, unsigned long long idArg)
{
    auto item {new QListWidgetItem{}};
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%R");
    MessageWidget* msgWidget{new MessageWidget{msg, ss.str().c_str()}};
    item->setSizeHint(msgWidget->sizeHint());
    ui->messageList->addItem(item);
    ui->messageList->setItemWidget(item, msgWidget);
}

