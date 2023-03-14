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
    //explicit ContactsWidget(const QString& name, unsigned long long id, std::pair<unsigned long long, QString> lastMessageInfo, QWidget *parent = nullptr);
    explicit ContactsWidget(Contact contact, QWidget *parent = nullptr);
    ~ContactsWidget();
    Contact& getContact();
    void setLastMessage(bool isAuthor, const QString& message);
private:
    Ui::ContactsWidget *ui;
    Contact contact_;
};

