#include "Mainwindow.h"
#include "./ui_Mainwindow.h"
#include <QStringListModel>
#include <QString>
#include <IoServiceWorker.h>
#include <QDateTime>
#include "NotificationWidget.h"

MainWindow::MainWindow(boost::asio::io_service& service, const std::string& hash, const QString& userNickname, bool isGuestAccount, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //Setting up ui components
    ui->setupUi(this);
    ui->searchLine->setClearButtonEnabled(true);
    ui->splitter->setSizes(QList<int>({1,500}));
    ui->splitter->setCollapsible(0, false);
    ui->splitter->setCollapsible(1, false);

    mediator_ = new Mediator(this);

    //Setting up contactsListWidget
    contactsListWidget.reset(new ContactsListWidget{mediator_});
    connect(&*contactsListWidget, &ContactsListWidget::loadChatInfo, this, &MainWindow::loadChatInfo);
    connect(&*contactsListWidget, &ContactsListWidget::cleanSearchLine, this, &MainWindow::cleanSearchLine);
    ui->contactsListLayout->addWidget(contactsListWidget.get());

    //Setting up messenger_ and retrieving contacts
    messenger_.reset(new Messenger<MainWindow>{service, hash, this, isGuestAccount});
    connect(&messenger_->signalHandler, &MessengerSignalHandler::deleteMessageRequest, this, &MainWindow::onDeleteMessageRequest);
    connect(&messenger_->signalHandler, &MessengerSignalHandler::editMessageRequest, this, &MainWindow::onEditMessageRequest);
    connect(&messenger_->signalHandler, &MessengerSignalHandler::deleteChatFromList, this, &MainWindow::onDeleteChatFromList);
    messenger_->initializeConnection();
    messenger_->setReceiveMessageCallback(&MainWindow::sendMessageToChat);
    while(messenger_->infoIsLoaded);
    contactsListWidget->addContacts(messenger_->getFriendList());
    connect(this, &MainWindow::createMessageInstanceSignal, this, &MainWindow::createMessageInstance);

    connect(mediator_, &Mediator::contextMenuSignal, this, &MainWindow::onContextMenuSlot);
    connect(mediator_, &Mediator::contextMenuMessageRemovalFromDbSignal, this, &MainWindow::onContextMenuMessageRemovalFromDbSlot);
    connect(mediator_, &Mediator::deleteChat, this, &MainWindow::onDeleteChat);

    settingsDialog_.reset(new SettingsDialog(this, userNickname));
    connect(&*settingsDialog_, &SettingsDialog::sendEmailForVerificationSignal, this, &MainWindow::onSendEmailForVerificationSignal);
    connect(&*settingsDialog_, &SettingsDialog::sendVerificationCodeSignal, this, &MainWindow::onSendVerificationCodeSignal);
    connect(&*settingsDialog_, &SettingsDialog::disableEmailAuthenticationSignal, this, &MainWindow::onDisableEmailAuthentication);
    connect(&*settingsDialog_, &SettingsDialog::deleteAccountSignal, this, &MainWindow::onDeleteAccount);
    connect(&*settingsDialog_, &SettingsDialog::changePassword, this, &MainWindow::onChangePassword);
    connect(&*settingsDialog_, &SettingsDialog::updateAvatar, this, &MainWindow::onUpdateAvatar);
    connect(&messenger_->signalHandler, &MessengerSignalHandler::sendCodeVerificationResult, &*settingsDialog_, &SettingsDialog::retrieveCodeVerificationResult);
    if(isGuestAccount){
        ui->settingsButton->setVisible(false);
    }
}

MainWindow::~MainWindow()
{
    IoServiceWorker::getInstance().getService().stop();
    delete ui;
}

//Method which retrieves messages from server (those messages are sent to Messenger.h and then callback invokes this method)
void MainWindow::sendMessageToChat(const MessageInfo &messageInfo)
{
    auto friendList{messenger_->getFriendList()};
    //auto contact{std::find_if(friendList.begin(), friendList.end(), [&](Contact& tmpContact){qDebug() << tmpContact.getId()<< " "<< id;return tmpContact.getId() == id;})};
    emit createMessageInstanceSignal(messageInfo);
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
    chatsMap[id].reset(new Chat{id, name, mediator_});
    connect(&(*(chatsMap[id])), &Chat::sendMessage, this, &MainWindow::sendMessage);
    connect(&(*(chatsMap[id])), &Chat::sendFile, this, &MainWindow::sendFile);
    connect(&(*(chatsMap[id])), &Chat::editMessageInDb, this, &MainWindow::onEditMessageInDb);
    if(currentFriendId!=0){
        chatsMap[currentFriendId]->setVisible(false);
        ui->rightLayout->replaceWidget(chatsMap[currentFriendId].get(), chatsMap[id].get());
    }else{
        ui->rightLayout->addWidget(chatsMap[id].get());
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

void MainWindow::processChatDeletion(unsigned long long chatId)
{
    auto chatIter = chatsMap.find(chatId);
    if (chatIter != chatsMap.end()) {
        // Loop through the widgets in the QVBoxLayout
        for (int i = 0; i < ui->rightLayout->count(); ++i) {
            QWidget* widget = ui->rightLayout->itemAt(i)->widget();
            if (widget == chatIter->second.get()) {
                ui->rightLayout->removeWidget(widget);
                widget->setParent(nullptr); // Or delete widget; if appropriate
                break;
            }
        }

        // Delete the chat from the map
        chatsMap.erase(chatId);
    }
    contactsListWidget->setCurrentFriendId(0);
    contactsListWidget->removeChat(chatId);
}

void MainWindow::updateLastMessageIfNecessary(unsigned long long friendId)
{
    auto lastMessage{chatsMap[friendId]->getLastMessage()};
    if(lastMessage.has_value()){
        contactsListWidget->setLastMessage(friendId, lastMessage.value().text, lastMessage.value().isAuthor);
    }else{
        contactsListWidget->setLastMessage(friendId, "", true);
    }
}

void MainWindow::sendMessage(const MessageInfo &msgInfo)
{
    contactsListWidget->setLastMessage(msgInfo.friendId, msgInfo.text, msgInfo.isAuthor);
    messenger_->sendMessage(msgInfo);
}

void MainWindow::createMessageInstance(const MessageInfo &msgInfo)
{
    auto id{msgInfo.friendId};
    auto tmpMsgInfo{msgInfo};
    auto currentDateTime{QDateTime::currentDateTime()};
    tmpMsgInfo.sentTime = currentDateTime.time().toString("hh:mm");
    if(chatsMap.find(id) == chatsMap.end()){
        chatsMap[id].reset(new Chat{id, msgInfo.senderName.c_str(), mediator_});
    }
    chatsMap[id]->receiveMessage(tmpMsgInfo, currentDateTime, false);
    auto friendWidget {contactsListWidget->findFriendById(id)};
    if(!friendWidget.has_value()){
        Contact contact{msgInfo.senderName.c_str(), id, {123, tmpMsgInfo.text}, {}};
        friendWidget = contactsListWidget->constructContact(contact);
    }
    friendWidget.value().second->setLastMessage(false, tmpMsgInfo.text);
    auto currentItem {contactsListWidget->getContactsWidget()->currentItem()};
    auto currentWidget{dynamic_cast<ContactsWidget*>(contactsListWidget->getContactsWidget()->itemWidget(currentItem))};
    if(QApplication::applicationState() == Qt::ApplicationInactive ||
        (ui->rightLayout->isEmpty() || (friendWidget.has_value() && friendWidget.value().second->getContact().getId()!=currentWidget->getContact().getId())))
    {
        popupNotification(tmpMsgInfo.text, friendWidget.value().second->getContact().getName(), id);
    }
}

void MainWindow::showAndActivate()
{
    showNormal();
    show();
    activateWindow();
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

void MainWindow::onContextMenuSlot(const MessageInfo &msgInfo)
{
    auto friendWidget {contactsListWidget->findFriendById(msgInfo.friendId)};
    if(friendWidget.has_value()){
        popupNotification(msgInfo.text, friendWidget.value().second->getContact().getName(), msgInfo.friendId, 5 * 1000); //30 secs
    }
}

void MainWindow::onContextMenuMessageRemovalFromDbSlot(const MessageInfo& msgInfo)
{
    messenger_->removeMessageFromDb(msgInfo);
    updateLastMessageIfNecessary(msgInfo.friendId);
}

void MainWindow::onDeleteMessageRequest(const QString &chatId, const QString &messageGuid)
{
    auto ULLChatId {chatId.toULongLong()};
    if(chatsMap.find(ULLChatId) == chatsMap.end()){
        messenger_->requestChatHistory(ULLChatId);
        messenger_->infoIsLoaded = true;
        while(messenger_->infoIsLoaded);
        auto chatHistory{messenger_->getChatHistory()};
        chatsMap[ULLChatId].reset(new Chat{ULLChatId, "123", mediator_});
        chatsMap[ULLChatId]->loadChatHistory(chatHistory);
    }

    chatsMap[ULLChatId]->onContextMenuMessageRemovalSignal({messageGuid, 0,0,"", "", true});
    updateLastMessageIfNecessary(ULLChatId);
}

void MainWindow::onEditMessageRequest(const QString &chatId, const QString &messageGuid, const QString &newText)
{
    if(chatsMap.find(chatId.toULongLong()) != chatsMap.end()){
        chatsMap[chatId.toULongLong()]->editMessageIfExists(messageGuid, newText);
    }
    contactsListWidget->setLastMessage(chatId.toULongLong(), newText, false);
}

void MainWindow::on_settingsButton_clicked()
{
    //open up the settings widget
    settingsDialog_->setCurrentEmail(messenger_->getEmail());
    settingsDialog_->showWindow();
}

void MainWindow::onSendEmailForVerificationSignal(const std::string &email)
{
    messenger_->sendEmailForCodeVerification(email);
}

void MainWindow::onSendVerificationCodeSignal(const std::string &code)
{
    messenger_->sendVerificationCode(code);
}

void MainWindow::onDisableEmailAuthentication()
{
    messenger_->disableEmailAuth();
}

void MainWindow::sendFile(const std::string& filePath, unsigned long long receiverId)
{
    messenger_->sendFile(filePath, receiverId);
}

void MainWindow::onEditMessageInDb(const MessageInfo &msgInfo)
{
    messenger_->editMessageInDb(msgInfo);
    contactsListWidget->setLastMessage(msgInfo.friendId, msgInfo.text, msgInfo.isAuthor);
}

void MainWindow::onDeleteAccount()
{
    messenger_->deleteAccount();
    QCoreApplication::quit();
}

void MainWindow::onChangePassword(const std::string &newPassword)
{
    messenger_->changePassword(newPassword);
}

void MainWindow::onUpdateAvatar(const std::string &photoStream)
{
    messenger_->updateAvatar(photoStream);
}

void MainWindow::onDeleteChat(unsigned long long chatId)
{
    messenger_->deleteChat(chatId);
    processChatDeletion(chatId);
}

void MainWindow::onDeleteChatFromList(unsigned long long chatId)
{
    processChatDeletion(chatId);
}

