#include "ContactsWidget.h"
#include "ui_ContactsWidget.h"

#include <QMessageBox>

ContactsWidget::ContactsWidget(Contact contact, Mediator *mediator, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContactsWidget),
    contact_{contact},
    mediator_{mediator}
{
    ui->setupUi(this);
    ui->userNickname->setText(contact.getName());
    ui->deleteButton->setIcon(QIcon(QCoreApplication::applicationDirPath() + "/Assets/trashIcon.png"));
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
        ui->lastMessage->update();
    }
}

void ContactsWidget::setContact(const Contact& contact)
{
    contact_ = contact;
}

void ContactsWidget::hideDeleteChatButton()
{
    ui->deleteButton->hide();
}

void ContactsWidget::on_deleteButton_clicked()
{
    QMessageBox messageBox;
    messageBox.setWindowTitle("Confirm Deletion");
    messageBox.setText("Are you sure you want to delete your account?");
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    messageBox.setDefaultButton(QMessageBox::No);
    messageBox.setIcon(QMessageBox::Warning);

    messageBox.setStyleSheet(R"(
        QMessageBox {
            background-color: #333333; /* Dark grey background */
            color: #DDDDDD; /* Light grey text for readability */
        }
        QLabel {
            color: #DDDDDD;
        }
        QPushButton {
            background-color: #555555; /* Medium grey for buttons */
            color: #FFFFFF;
            border: none;
            border-radius: 4px;
            padding: 6px 20px;
            margin: 4px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #6E6E6E; /* Lighter grey for hover state */
        }
        QPushButton:pressed {
            background-color: #4D4D4D; /* Slightly darker grey for pressed state */
        }
    )");

    int result = messageBox.exec();
    if (result == QMessageBox::Yes) {
        emit deleteChatSignal(contact_.getId());
    }
}
