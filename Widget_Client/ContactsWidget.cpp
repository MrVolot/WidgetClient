#include "ContactsWidget.h"
#include "ui_ContactsWidget.h"

ContactsWidget::ContactsWidget(const QString& name, unsigned long long id, std::pair<unsigned long long, QString> lastMessageInfo, QWidget *parent) :
    QWidget(parent),
    name_{name},
    id_{id},
    lastMessageInfo_{lastMessageInfo},
    ui(new Ui::ContactsWidget)
{
    ui->setupUi(this);
    ui->userNickname->setText(name);
    QString sender{"You: "};
    if(id == lastMessageInfo.first){
        sender = name + ": ";
    }
    sender.append(lastMessageInfo.second);
    ui->lastMessage->setText(sender);
}

ContactsWidget::~ContactsWidget()
{
    delete ui;
}

void ContactsWidget::setLastMessage(bool isAuthor, const QString &message)
{
    QString sender{"You: "};
    if(!isAuthor){
        sender = name_ + ": ";
    }
    sender.append(message);
    ui->lastMessage->setText(sender);
}
QString &ContactsWidget::getName()
{
    return name_;
}

unsigned long long ContactsWidget::getId()
{
    return id_;
}
