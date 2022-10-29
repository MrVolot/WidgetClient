#pragma once

#include <QWidget>

namespace Ui {
class Chat;
}

class Chat : public QWidget
{
    Q_OBJECT

public:
    explicit Chat(const QString& nameArg, QWidget *parent = nullptr);
    ~Chat();

private slots:
    void on_sendMsgBtn_clicked();

private:
    Ui::Chat *ui;
    std::string name;
signals:
    void sendMessage(const QString& msg, long long id);
};

