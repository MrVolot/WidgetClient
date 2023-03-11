#include "Mainwindow.h"
#include "./ui_Mainwindow.h"
#include <QStringListModel>
#include <QString>
#include <IoServiceWorker.h>
#include <QDateTime>
#include <QSystemTrayIcon>

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

    notificationWidget = new NotificationWidget();
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
        ContactsWidget* contact{new ContactsWidget{contactItem.getName(), contactItem.getId(), contactItem.getLastMessageInfo()}};
        item->setSizeHint(contact->sizeHint());
        ui->contactListWidget->addItem(item);
        ui->contactListWidget->setItemWidget(item, contact);
    }
}

//Function used to get widget related to specified friend
ContactsWidget* MainWindow::findFriendById(unsigned long long id)
{
    for (int i = 0; i < ui->contactListWidget->count(); ++i) {
        QListWidgetItem* item =  ui->contactListWidget->item(i);
        ContactsWidget* contactsWidget = qobject_cast<ContactsWidget*>( ui->contactListWidget->itemWidget(item));
        if (contactsWidget && contactsWidget->getId() == id) {
            return contactsWidget;
        }
    }
    return nullptr;
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
    auto contact{dynamic_cast<ContactsWidget*>(ui->contactListWidget->itemWidget(item))};
    if(contact->getId() == currentFriend){
        return;
    }
    messenger_->requestChatHistory(contact->getId());
    messenger_->infoIsLoaded = true;
    while(messenger_->infoIsLoaded);
    auto chatHistory{messenger_->getChatHistory()};
    chatsMap[contact->getId()].reset(new Chat{contact->getName(), contact->getId()});
    connect(&(*(chatsMap[contact->getId()])), &Chat::sendMessage, this, &MainWindow::sendMessage);
    if(currentFriend!=0){
        chatsMap[currentFriend]->setVisible(false);
        ui->chatLayout->replaceWidget(chatsMap[currentFriend].get(), chatsMap[contact->getId()].get());
    }else{
        ui->chatLayout->addWidget(chatsMap[contact->getId()].get());
    }
    chatsMap[contact->getId()]->loadChatHistory(chatHistory);
    currentFriend = contact->getId();
}

void MainWindow::createMessageInstance(const QString &msg, unsigned long long id)
{
    if(chatsMap.find(id) != chatsMap.end()){
        chatsMap[id]->receiveMessage(msg, QDateTime::currentDateTime(), false, false);
    }
    auto friendWidget {findFriendById(id)};
    if(friendWidget!=nullptr){
        friendWidget->setLastMessage(false, msg);
    }
    notificationWidget->showNotification(msg);
}

