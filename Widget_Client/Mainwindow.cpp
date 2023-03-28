#include "Mainwindow.h"
#include "./ui_Mainwindow.h"
#include <QStringListModel>
#include <QString>
#include <IoServiceWorker.h>
#include <QDateTime>

MainWindow::MainWindow(boost::asio::io_service& service, const std::string& hash, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->splitter->setSizes(QList<int>({1,500}));
    ui->splitter->setCollapsible(0, false);
    ui->splitter->setCollapsible(1, false);
    messenger_.reset(new Messenger<MainWindow>{service, hash, this});
    messenger_->initializeConnection();
    messenger_->setReceiveMessageCallback(&MainWindow::sendMessageToChat);
    while(messenger_->infoIsLoaded);
    pushFriendListToGui(messenger_->getFriendList());
    connect(this, &MainWindow::createMessageInstanceSignal, this, &MainWindow::createMessageInstance);
}

MainWindow::~MainWindow()
{
    IoServiceWorker::getInstance().getService().stop();
    delete ui;
}

//Method which retrieves messages from server (those messages are sent to Messenger.h and then callback invokes this method)
void MainWindow::sendMessageToChat(const QString &msg, unsigned long long id)
{
    auto friendList{messenger_->getFriendList()};
    auto contact{std::find_if(friendList.begin(), friendList.end(), [&](Contact& tmpContact){qDebug() << tmpContact.getId()<< " "<< id;return tmpContact.getId() == id;})};
    emit createMessageInstanceSignal(msg, id);
}

//Function used to display all friends that were loaded from DB
void MainWindow::pushFriendListToGui(std::vector<Contact> friendList)
{
    for(auto& contactItem : friendList){
        auto item {new QListWidgetItem{}};
        ContactsWidget* contact{new ContactsWidget{contactItem}};
        item->setSizeHint(contact->sizeHint());
        ui->contactListWidget->addItem(item);
        ui->contactListWidget->setItemWidget(item, contact);
    }
}

//Function used to get widget related to specified friend
std::optional<std::pair<QListWidgetItem*, ContactsWidget*>> MainWindow::findFriendById(unsigned long long id)
{
    for (int i = 0; i < ui->contactListWidget->count(); ++i) {
        QListWidgetItem* item =  ui->contactListWidget->item(i);
        ContactsWidget* contactsWidget = qobject_cast<ContactsWidget*>( ui->contactListWidget->itemWidget(item));
        if (contactsWidget && contactsWidget->getContact().getId() == id) {
            return std::make_pair(item, contactsWidget);
        }
    }
    return {};
}

void MainWindow::loadChatInfo(const QString &name, unsigned long long id)
{
    if(currentFriend == id){
        return;
    }
    messenger_->requestChatHistory(id);
    messenger_->infoIsLoaded = true;
    while(messenger_->infoIsLoaded);
    auto chatHistory{messenger_->getChatHistory()};
    chatsMap[id].reset(new Chat{name, id});
    connect(&*chatsMap[id], &Chat::finalizeMessageReminder, this, &MainWindow::finalizeMessageReminder);
    connect(&(*(chatsMap[id])), &Chat::sendMessage, this, &MainWindow::sendMessage);
    if(currentFriend!=0){
        chatsMap[currentFriend]->setVisible(false);
        ui->chatLayout->replaceWidget(chatsMap[currentFriend].get(), chatsMap[id].get());
    }else{
        ui->chatLayout->addWidget(chatsMap[id].get());
    }
    chatsMap[id]->loadChatHistory(chatHistory);
    currentFriend = id;
}

void MainWindow::popupNotification(const QString &msg, const QString &friendName, unsigned long long id, unsigned long long timeout)
{
    NotificationWidget* notificationWidget{new NotificationWidget(msg, id, friendName)};
    connect(notificationWidget, &NotificationWidget::showMainWindow, this, &MainWindow::showAndActivate);
    connect(notificationWidget, &NotificationWidget::reactOnNotification, this, &MainWindow::reactOnNotification);
    QTimer::singleShot(timeout, notificationWidget, &NotificationWidget::showNotification);
}

void MainWindow::sendMessage(const QString &msg, unsigned long long id)
{
    auto selectedItem {ui->contactListWidget->currentItem()};
    auto contact{dynamic_cast<ContactsWidget*>(ui->contactListWidget->itemWidget(selectedItem))};
    contact->setLastMessage(true, msg);
    messenger_->sendMessage(std::to_string(id), msg.toStdString());
}


void MainWindow::on_contactListWidget_itemClicked(QListWidgetItem *item)
{
    auto contact{dynamic_cast<ContactsWidget*>(ui->contactListWidget->itemWidget(item))->getContact()};
    if(contact.getId() == currentFriend){
        return;
    }
    loadChatInfo(contact.getName(),contact.getId());
}

void MainWindow::createMessageInstance(const QString &msg, unsigned long long id)
{
    if(chatsMap.find(id) != chatsMap.end()){
        chatsMap[id]->receiveMessage(msg, QDateTime::currentDateTime(), false, false);
    }
    auto friendWidget {findFriendById(id)};
    if(friendWidget.has_value()){
        friendWidget.value().second->setLastMessage(false, msg);
    }
    auto currentItem {ui->contactListWidget->currentItem()};
    auto currentWidget{dynamic_cast<ContactsWidget*>(ui->contactListWidget->itemWidget(currentItem))};
    if(QApplication::applicationState() == Qt::ApplicationInactive ||
      (ui->chatLayout->isEmpty() || (friendWidget.has_value() && friendWidget.value().second->getContact().getId()!=currentWidget->getContact().getId())))
    {
        popupNotification(msg, friendWidget.value().second->getContact().getName(), id);
    }
}

void MainWindow::showAndActivate()
{
    showNormal();
    show();
    activateWindow();
}

void MainWindow::reactOnNotification(unsigned long long id)
{
    auto friendWidget {findFriendById(id)};
    if(friendWidget.has_value()){
        ui->contactListWidget->setCurrentItem(friendWidget.value().first);
        loadChatInfo(friendWidget.value().second->getContact().getName(),friendWidget.value().second->getContact().getId());
    }
}

void MainWindow::finalizeMessageReminder(const QString &msg, unsigned long long id, unsigned long long timeout)
{
    auto friendWidget {findFriendById(id)};
    if(friendWidget.has_value()){
        popupNotification(msg, friendWidget.value().second->getContact().getName(), id);
    }
}

