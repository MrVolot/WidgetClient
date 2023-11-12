#pragma once
#include <QString>

class MessageInfo{
public:
    //TODO: refactor to have personalId and friendId
    QString messageGuid;
    unsigned long long senderId;
    unsigned long long receiverId;
    unsigned long long friendId;
    QString text;
    QString sentTime;
    bool isAuthor;
    std::string senderName;

    MessageInfo(QString messageGuid, unsigned long long senderId, unsigned long long receiverId, const QString& text, const QString& sentTime, bool isAuthor, const std::string& senderName = ""):
        messageGuid(messageGuid),
        senderId(senderId),
        receiverId(receiverId),
        text(text),
        sentTime(sentTime),
        isAuthor(isAuthor),
        senderName(senderName)
    {
        if (isAuthor) {
            friendId = receiverId;
        } else {
            friendId = senderId;
        }
    }
};
