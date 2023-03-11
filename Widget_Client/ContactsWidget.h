#pragma once

#include <QWidget>

namespace Ui {
class ContactsWidget;
}

class ContactsWidget : public QWidget
{
    Q_OBJECT
    QString name_;
    unsigned long long id_;
    std::pair<unsigned long long, QString> lastMessageInfo_;
public:
    explicit ContactsWidget(const QString& name, unsigned long long id, std::pair<unsigned long long, QString> lastMessageInfo, QWidget *parent = nullptr);
    QString& getName();
    unsigned long long getId();
    ~ContactsWidget();
    void setLastMessage(bool isAuthor, const QString& message);
private:
    Ui::ContactsWidget *ui;
};

