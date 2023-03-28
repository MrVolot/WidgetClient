#pragma once

#include <MessageWidget.h>
#include <QWidget>
#include <QDateTime>

namespace Ui {
class Chat;
}

class Chat : public QWidget
{
    Q_OBJECT

public:
    explicit Chat(const QString& nameArg, unsigned long long idArg, QWidget *parent = nullptr);
    void receiveMessage(const QString& msg, const QDateTime& sentTime, bool createDateWidget, bool isAuthor = true);
    ~Chat();
    void loadChatHistory(std::vector<std::map<std::string, QString>>& chatHistory);
    QString getCurrentTime();

private slots:
    void on_sendMsgBtn_clicked();
    void on_msgFiled_returnPressed();

private:
    Ui::Chat *ui;
    QString name;
    unsigned long long id;
    std::vector<MessageWidget*> vectorOfMessages;
    QDateTime lastMessageDateTime;

    void processMessage(const QString &msg, bool isAuthor);
    int getClosestPunctuationMarkPosition(const QString &msg, bool isLeftToRight);
    void splitIntoMessages(const QString &msg, bool isAuthor);
    bool hasSpaces(const QString& str);
    QString createWrap(const QString& str);
    std::chrono::system_clock::time_point getChronoTime(const std::string& timeStr);
    void createAndPushMessageWidget(const QString &msg, bool isAuthor);
signals:
    void sendMessage(const QString& msg, unsigned long long id);
    void finalizeMessageReminder(const QString &msg, unsigned long long id, unsigned long long timeout = 0);
public slots:
    void proceedMessageReminder(const QString &msg);
};

