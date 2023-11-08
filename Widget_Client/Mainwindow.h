#pragma once

#include <QListWidgetItem>
#include <QMainWindow>
#include "Messenger.h"
#include "Chat.h"
#include <optional>
#include "ContactsListWidget.h"
#include "Mediator.h"
#include "SettingsDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(boost::asio::io_service& service, const std::string& hash, const QString& userNickname, bool isGuestAccount = false, QWidget *parent = nullptr);
    ~MainWindow();
private:
    Ui::MainWindow *ui;
    std::shared_ptr<Messenger<MainWindow>> messenger_;
    std::map<unsigned long long, std::unique_ptr<Chat>> chatsMap;
    std::mutex mtx;
    std::condition_variable cv;
    unsigned long long currentFriend=0;
    std::unique_ptr<ContactsListWidget> contactsListWidget;
    Mediator *mediator_;
    std::unique_ptr<SettingsDialog> settingsDialog_;

    bool canProceed{false};
    void sendMessageToChat(const MessageInfo &messageInfo);
    void popupNotification(const QString &msg, const QString &friendName, unsigned long long id, unsigned long long timeout=0);
private slots:
    void loadChatInfo(Contact& contact);
    void sendMessage(const MessageInfo &msgInfo);
    void createMessageInstance(const MessageInfo &msgInfo);
    void showAndActivate();
    void on_searchLine_returnPressed();
    void on_searchLine_textChanged(const QString& text);
    void cleanSearchLine();
    void onContextMenuSlot(const MessageInfo &msgInfo);
    void onContextMenuMessageRemovalFromDbSlot(const MessageInfo& msgInfo);
    void onDeleteMessageRequest(const QString& chatId, const QString& messageGuid);
    void onEditMessageRequest(const QString& chatId, const QString& messageGuid, const QString& newText);
    void on_settingsButton_clicked();
    void onSendEmailForVerificationSignal(const std::string& email);
    void onSendVerificationCodeSignal(const std::string& code);
    void onDisableEmailAuthentication();
    void sendFile(const std::string& filePath, unsigned long long receiverId);
    void onEditMessageInDb(const MessageInfo &msgInfo);
    void onDeleteAccount();
    void onChangePassword(const std::string& newPassword);
    void onUpdateAvatar(const std::string& photoStream);
signals:
    void createMessageInstanceSignal(const MessageInfo &msgInfo);
};
