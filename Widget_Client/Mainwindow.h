#pragma once

#include <QListWidgetItem>
#include <QMainWindow>
#include "boost/asio.hpp"
#include "Messenger.h"
#include "Chat.h"

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
    std::unique_ptr<Chat> chat;
private slots:
    void pushFriendListToGui(std::vector<Contact> friendList);
    void on_listView_clicked(const QModelIndex &index);
    void sendMessage(const QString& msg, long long id);
    void on_contactListWidget_itemClicked(QListWidgetItem *item);
};
