#pragma once
#include <QObject>

class MessengerSignalHandler: public QObject
{
    Q_OBJECT
public:
    MessengerSignalHandler(QObject* parent = nullptr);
signals:
    void deleteMessageRequest(const QString& chatId, const QString& messageGuid);
    void sendCodeVerificationResult(bool result);
    void editMessageRequest(const QString& chatId, const QString& messageGuid, const QString& newText);
    void deleteChatFromList(unsigned long long chatId);
    void processFileSignal(const QString& fileName, const std::string &fileStream);
};

