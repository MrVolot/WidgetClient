#pragma once

#include <QWidget>
#include "Contact.h"

namespace Ui {
class ContactsWidget;
}

class ContactsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContactsWidget(Contact contact, QWidget *parent = nullptr);
    ~ContactsWidget();
    Contact& getContact();
    void setLastMessage(bool isAuthor, const QString& message);
    void setContact(const Contact& contact);
private:
    Ui::ContactsWidget *ui;
    Contact contact_;
};

