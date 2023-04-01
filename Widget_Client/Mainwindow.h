#pragma once

#include <QListWidgetItem>
#include <QMainWindow>
#include "Messenger.h"
#include "Chat.h"
#include <optional>
#include "ContactsListWidget.h"

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
    std::unique_ptr<ContactsListWidget> contactsListWidget;

    bool canProceed{false};
    void sendMessageToChat(const QString& msg, unsigned long long id);
    void popupNotification(const QString &msg, const QString &friendName, unsigned long long id, unsigned long long timeout=0);
private slots:
    void loadChatInfo(Contact& contact);
    void sendMessage(const QString& msg, unsigned long long id);
    void createMessageInstance(const QString& msg, unsigned long long id);
    void showAndActivate();
    void finalizeMessageReminder(const QString &msg, unsigned long long id, unsigned long long timeout=0);
    void on_searchLine_returnPressed();
    void on_searchLine_textChanged(const QString& text);
    void cleanSearchLine();
signals:
    void createMessageInstanceSignal(const QString& msg, unsigned long long id);
};
