#pragma once

#include <MessageWidget.h>
#include <QWidget>

namespace Ui {
class Chat;
}

class Chat : public QWidget
{
    Q_OBJECT

public:
    explicit Chat(const QString& nameArg, unsigned long long idArg, QWidget *parent = nullptr);
    void receiveMessage(const QString& msg, unsigned long long idArg, bool isAuthor = true);
    ~Chat();

private slots:
    void on_sendMsgBtn_clicked();
    void on_msgFiled_returnPressed();

private:
    Ui::Chat *ui;
    QString name;
    unsigned long long id;
signals:
    void sendMessage(const QString& msg, unsigned long long id);
};

