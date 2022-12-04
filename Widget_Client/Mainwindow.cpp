#include "Mainwindow.h"
#include "./ui_Mainwindow.h"
#include "RegisterDialog.h"
#include <QStringListModel>
#include <QString>
#include <IoServiceWorker.h>

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

void MainWindow::sendMessageToChat(const QString &msg, unsigned long long id)
{
    auto friendList{messenger_->getFriendList()};
    auto contact{std::find_if(friendList.begin(), friendList.end(), [&](Contact& tmpContact){qDebug() << tmpContact.getId()<< " "<< id;return tmpContact.getId() == id;})};
    emit createMessageInstanceSignal(msg, contact->getId());
}

void MainWindow::pushFriendListToGui(std::vector<Contact> friendList)
{
    for(auto& contactItem : friendList){
        auto item {new QListWidgetItem{}};
        ContactsWidget* contact{new ContactsWidget{contactItem.getName(), contactItem.getId()}};
        item->setSizeHint(contact->sizeHint());
        ui->contactListWidget->addItem(item);
        ui->contactListWidget->setItemWidget(item, contact);
    }
}

void MainWindow::sendMessage(const QString &msg, unsigned long long id)
{
    messenger_->sendMessage(std::to_string(id), msg.toStdString());
}


void MainWindow::on_contactListWidget_itemClicked(QListWidgetItem *item)
{
    auto contact{dynamic_cast<ContactsWidget*>(ui->contactListWidget->itemWidget(item))};
    chat.reset(new Chat{contact->getName(), contact->getId()});
    connect(&(*chat), &Chat::sendMessage, this, &MainWindow::sendMessage);
    ui->chatLayout->addWidget(chat.get());

}

void MainWindow::createMessageInstance(const QString &msg, unsigned long long id)
{
    chat->receiveMessage(msg, id, false);
}

