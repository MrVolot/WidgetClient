#pragma once
#include <QString>

struct MessageInfo{
    //TODO: refactor to have personalId and friendId
    QString messageGuid;
    unsigned long long senderId;
    unsigned long long receiverId;
    unsigned long long friendId;
    QString text;
    QString sentTime;
    bool isAuthor;

    MessageInfo(QString messageGuid, unsigned long long senderId, unsigned long long receiverId, const QString& text, const QString& sentTime, bool isAuthor):
        messageGuid(messageGuid),
        senderId(senderId),
        receiverId(receiverId),
        text(text),
        sentTime(sentTime),
        isAuthor(isAuthor)
    {
        if (isAuthor) {
            friendId = receiverId;
        } else {
            friendId = senderId;
        }
    }
};
