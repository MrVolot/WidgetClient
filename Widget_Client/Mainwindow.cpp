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
    messenger_.reset(new Messenger<MainWindow>{service, hash, this});
    messenger_->initializeConnection();
    while(messenger_->infoIsLoaded);
    pushFriendListToGui(messenger_->getFriendList());
}

MainWindow::~MainWindow()
{
    IoServiceWorker::getInstance().getService().stop();
    delete ui;
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


void MainWindow::on_listView_clicked(const QModelIndex &index)
{
    //std::unique_ptr<Chat> chatik{new Chat};
    chat.reset(new Chat{index.data().toString()});
    connect(&(*chat), &Chat::sendMessage, this, &MainWindow::sendMessage);
    ui->chatLayout->addWidget(chat.get());
}

void MainWindow::sendMessage(const QString &msg, long long id)
{
    messenger_->sendMessage(std::to_string(id), msg.toStdString());
}


void MainWindow::on_contactListWidget_itemClicked(QListWidgetItem *item)
{
    auto contact{dynamic_cast<ContactsWidget*>(ui->contactListWidget->itemWidget(item))};
    chat.reset(new Chat{QString::QString(contact->getName())});
    connect(&(*chat), &Chat::sendMessage, this, &MainWindow::sendMessage);
    ui->chatLayout->addWidget(chat.get());
}

