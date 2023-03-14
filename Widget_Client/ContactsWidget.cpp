#include "ContactsWidget.h"
#include "ui_ContactsWidget.h"

ContactsWidget::ContactsWidget(Contact contact, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContactsWidget),
    contact_{contact}
{
    ui->setupUi(this);
    ui->userNickname->setText(contact.getName());
    QString sender{"You: "};
    if(contact.getId() == contact.getLastMessageInfo().first){
        sender = contact.getName() + ": ";
    }
    sender.append(contact.getLastMessageInfo().second);
    ui->lastMessage->setText(sender);
}

ContactsWidget::~ContactsWidget()
{
    delete ui;
}

Contact &ContactsWidget::getContact()
{
    return contact_;
}

void ContactsWidget::setLastMessage(bool isAuthor, const QString &message)
{
    QString sender{"You: "};
    if(!isAuthor){
        sender = contact_.getName() + ": ";
    }
    sender.append(message);
    ui->lastMessage->setText(sender);
}
