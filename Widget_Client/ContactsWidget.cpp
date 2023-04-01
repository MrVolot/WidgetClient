#include "ContactsWidget.h"
#include "ui_ContactsWidget.h"

ContactsWidget::ContactsWidget(Contact contact, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContactsWidget),
    contact_{contact}
{
    ui->setupUi(this);
    ui->userNickname->setText(contact.getName());
    setLastMessage(contact.getId() != contact.getLastMessageInfo().first, contact.getLastMessageInfo().second);
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
    if(!message.isEmpty()){
        QString sender{"You: "};
        if(!isAuthor){
            sender = contact_.getName() + ": ";
        }
        sender.append(message);
        ui->lastMessage->setText(sender);
    }
}

void ContactsWidget::setContact(const Contact& contact)
{
    contact_ = contact;
}
