#pragma once

#include <Mediator.h>
#include <QWidget>
#include "Contact.h"

namespace Ui {
class ContactsWidget;
}

class ContactsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContactsWidget(Contact contact, Mediator *mediator, QWidget *parent = nullptr);
    ~ContactsWidget();
    Contact& getContact();
    void setLastMessage(bool isAuthor, const QString& message);
    void setContact(const Contact& contact);
    void hideDeleteChatButton();
private slots:
    void on_deleteButton_clicked();

private:
    Ui::ContactsWidget *ui;
    Contact contact_;
    Mediator *mediator_;
signals:
    void deleteChatSignal(unsigned long long id);
};

