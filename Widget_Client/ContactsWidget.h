#pragma once

#include <QWidget>

namespace Ui {
class ContactsWidget;
}

class ContactsWidget : public QWidget
{
    Q_OBJECT
    QString name_;
    long long id_;
public:
    explicit ContactsWidget(const QString& name, long long id, QWidget *parent = nullptr);
    QString& getName();
    long long getId();
    ~ContactsWidget();

private:
    Ui::ContactsWidget *ui;
};

