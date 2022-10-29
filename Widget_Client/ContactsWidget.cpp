#include "ContactsWidget.h"
#include "ui_ContactsWidget.h"

ContactsWidget::ContactsWidget(const QString& name, long long id, QWidget *parent) :
    name_{name},
    id_{id},
    QWidget(parent),
    ui(new Ui::ContactsWidget)
{
    ui->setupUi(this);
    ui->userNickname->setText(name);
}

ContactsWidget::~ContactsWidget()
{
    delete ui;
}
QString &ContactsWidget::getName()
{
    return name_;
}

long long ContactsWidget::getId()
{
    return id_;
}
