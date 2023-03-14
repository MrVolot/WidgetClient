#pragma once

#include <QListWidgetItem>
#include <QMainWindow>
#include "boost/asio.hpp"
#include "Messenger.h"
#include "Chat.h"
#include "NotificationWidget.h"
#include <optional>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(boost::asio::io_service& service, const std::string& hash, QWidget *parent = nullptr);
    ~MainWindow();
private:
    Ui::MainWindow *ui;
    std::shared_ptr<Messenger<MainWindow>> messenger_;
    std::map<unsigned long long, std::unique_ptr<Chat>> chatsMap;
    std::mutex mtx;
    std::condition_variable cv;
    unsigned long long currentFriend=0;
//    NotificationWidget* notificationWidget;

    bool canProceed{false};
    void sendMessageToChat(const QString& msg, unsigned long long id);
    void pushFriendListToGui(std::vector<Contact> friendList);
    std::optional<std::pair<QListWidgetItem*, ContactsWidget*>> findFriendById(unsigned long long id);
    void loadChatInfo(const QString &name, unsigned long long id);

private slots:
    void sendMessage(const QString& msg, unsigned long long id);
    void on_contactListWidget_itemClicked(QListWidgetItem *item);
    void createMessageInstance(const QString& msg, unsigned long long id);
    void showAndActivate();
    void reactOnNotification(unsigned long long id);
signals:
    void createMessageInstanceSignal(const QString& msg, unsigned long long id);
};
