#pragma once

#include "MessageWidget.h"
#include <QWidget>
#include <QDateTime>

namespace Ui {
class Chat;
}

class Chat : public QWidget
{
    Q_OBJECT
    friend class ChatFunctionalTests;
public:
    explicit Chat(unsigned long long friendId, const QString& friendName, Mediator *mediator, QWidget *parent = nullptr);
    void receiveMessage(const MessageInfo& msgInfo, const QDateTime& sentTime, bool createDateWidget);
    ~Chat();
    void loadChatHistory(std::vector<std::map<std::string, QString>>& chatHistory);
    QString getCurrentTime();
    void editMessageIfExists(const QString& messageGuid, const QString& newText);
private slots:
    void on_sendMsgBtn_clicked();
    void on_msgFiled_returnPressed();

private:
    Ui::Chat *ui;
    unsigned long long friendId_;
    QString friendName_;
    std::vector<MessageWidget*> vectorOfMessages;
    QDateTime lastMessageDateTime;
    Mediator *mediator_;
    std::unordered_map<std::string, int> messagesMap;
    MessageInfo currentlyEditingMessage_;

    void processMessage(const QString &msg, bool isAuthor);
    int getClosestPunctuationMarkPosition(const QString &msg, bool isLeftToRight);
    void splitIntoMessages(const QString &msg, bool isAuthor);
    bool hasSpaces(const QString& str);
    QString createWrap(const QString& str);
    std::chrono::system_clock::time_point getChronoTime(const std::string& timeStr);
    void createAndPushMessageWidget(const QString &msg, bool isAuthor);
signals:
    void sendMessage(const MessageInfo& messageInfo);
    void sendFile(const std::string& filePath, unsigned long long receiverId);
    void editMessageInDb(const MessageInfo& messageInfo);
public slots:
    void onContextMenuMessageRemovalSignal(const MessageInfo & msgInfo);
    void onEditMessageRequested(const MessageInfo & msgInfo);
protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

