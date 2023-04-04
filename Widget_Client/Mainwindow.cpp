#include "Mainwindow.h"
#include "./ui_Mainwindow.h"
#include <QStringListModel>
#include <QString>
#include <IoServiceWorker.h>
#include <QDateTime>
#include "NotificationWidget.h"

MainWindow::MainWindow(boost::asio::io_service& service, const std::string& hash, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //Setting up ui components
    ui->setupUi(this);
    ui->searchLine->setClearButtonEnabled(true);
    ui->splitter->setSizes(QList<int>({1,500}));
    ui->splitter->setCollapsible(0, false);
    ui->splitter->setCollapsible(1, false);

    //Setting up contactsListWidget
    contactsListWidget.reset(new ContactsListWidget{});
    connect(&*contactsListWidget, &ContactsListWidget::loadChatInfo, this, &MainWindow::loadChatInfo);
    connect(&*contactsListWidget, &ContactsListWidget::cleanSearchLine, this, &MainWindow::cleanSearchLine);
    ui->contactsLayout->addWidget(contactsListWidget.get());

    //Setting up messenger_ and retrieving contacts
    messenger_.reset(new Messenger<MainWindow>{service, hash, this});
    messenger_->initializeConnection();
    messenger_->setReceiveMessageCallback(&MainWindow::sendMessageToChat);
    while(messenger_->infoIsLoaded);
    contactsListWidget->addContacts(messenger_->getFriendList());
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

void MainWindow::loadChatInfo(Contact& contact)
{
    auto id{contact.getId()};
    auto name{contact.getName()};
    auto currentFriendId{contactsListWidget->getCurrentFriendId()};
    if(currentFriendId == id){
        return;
    }
    messenger_->requestChatHistory(id);
    messenger_->infoIsLoaded = true;
    while(messenger_->infoIsLoaded);
    auto chatHistory{messenger_->getChatHistory()};
    chatsMap[id].reset(new Chat{name, id});
    connect(&*chatsMap[id], &Chat::finalizeMessageReminder, this, &MainWindow::finalizeMessageReminder);
    connect(&(*(chatsMap[id])), &Chat::sendMessage, this, &MainWindow::sendMessage);
    if(currentFriendId!=0){
        chatsMap[currentFriendId]->setVisible(false);
        ui->chatLayout->replaceWidget(chatsMap[currentFriendId].get(), chatsMap[id].get());
    }else{
        ui->chatLayout->addWidget(chatsMap[id].get());
    }
    chatsMap[id]->loadChatHistory(chatHistory);
    contactsListWidget->setCurrentFriendId(id);
}

void MainWindow::popupNotification(const QString &msg, const QString &friendName, unsigned long long id, unsigned long long timeout)
{
    NotificationWidget* notificationWidget{new NotificationWidget(msg, id, friendName)};
    connect(notificationWidget, &NotificationWidget::showMainWindow, this, &MainWindow::showAndActivate);
    connect(notificationWidget, &NotificationWidget::reactOnNotification, contactsListWidget.get(), &ContactsListWidget::setAndOpenChat);
    QTimer::singleShot(timeout, notificationWidget, &NotificationWidget::showNotification);
}

void MainWindow::sendMessage(const QString &msg, unsigned long long id)
{
    auto selectedItem {contactsListWidget->getContactsWidget()->currentItem()};
    auto contact{dynamic_cast<ContactsWidget*>(contactsListWidget->getContactsWidget()->itemWidget(selectedItem))};
    contact->setLastMessage(true, msg);
    messenger_->sendMessage(id, msg.toStdString());
}

void MainWindow::createMessageInstance(const QString &msg, unsigned long long id)
{
    if(chatsMap.find(id) != chatsMap.end()){
        chatsMap[id]->receiveMessage(msg, QDateTime::currentDateTime(), false, false);
    }
    auto friendWidget {contactsListWidget->findFriendById(id)};
    if(friendWidget.has_value()){
        friendWidget.value().second->setLastMessage(false, msg);
    }
    auto currentItem {contactsListWidget->getContactsWidget()->currentItem()};
    auto currentWidget{dynamic_cast<ContactsWidget*>(contactsListWidget->getContactsWidget()->itemWidget(currentItem))};
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

void MainWindow::finalizeMessageReminder(const QString &msg, unsigned long long id, unsigned long long timeout)
{
    auto friendWidget {contactsListWidget->findFriendById(id)};
    if(friendWidget.has_value()){
        popupNotification(msg, friendWidget.value().second->getContact().getName(), id, 30 * 1000); //30 secs
    }
}


void MainWindow::on_searchLine_returnPressed()
{
    if(ui->searchLine->text().isEmpty()){
        return;
    }
    messenger_->tryFindByLogin(ui->searchLine->text());
    while(!messenger_->possibleContactsLoaded);
    messenger_->possibleContactsLoaded = false;
    auto possibleContacts {messenger_->getPossibleContacts()};
    messenger_->cleanPossibleContacts();
    contactsListWidget->showTemporaryContactsWidget(possibleContacts);
}

void MainWindow::on_searchLine_textChanged(const QString& text)
{
    // If the search field is empty, switch back to the main contacts widget
    if (text.isEmpty()) {
        qDebug()<<"0 text";
        contactsListWidget->showMainContactsWidget();
    }
}

void MainWindow::cleanSearchLine()
{
    ui->searchLine->clear();
}
