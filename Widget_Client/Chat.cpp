#include "Chat.h"
#include "FileDropDialog.h"
#include "qevent.h"
#include "ui_Chat.h"
#include "MessagesDateWidget.h"
#include <sstream>
#include <iomanip>
#include <iterator>
#include <QDate>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <QMimeData>
#include <QFileInfo>
#include <QUrl>

Chat::Chat(unsigned long long friendId, const QString& friendName, Mediator *mediator, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Chat),
    friendId_{friendId},
    friendName_{friendName},
    mediator_{mediator},
    currentlyEditingMessage_{"", 0, 0, "", "", true}
{
    setAcceptDrops(true);
    ui->setupUi(this);
    ui->receiverName->setText(friendName_);
    ui->msgFiled->setClearButtonEnabled(true);
    ui->messageList->setStyleSheet("QListWidget::item:hover {background: transparent;} "
                                   "QListWidget::item:selected:!active {background: transparent;} ");
    ui->messageList->setFocusPolicy(Qt::NoFocus);

    connect(mediator_, &Mediator::contextMenuMessageRemovalSignal, this, &Chat::onContextMenuMessageRemovalSignal);
    connect(mediator_, &Mediator::editMessageSignal, this, &Chat::onEditMessageRequested);
    ui->cancelEditButton->setVisible(false);
    ui->cancelEditButton->setIcon(QIcon(QCoreApplication::applicationDirPath() + "/Assets/cancelIcon.png"));
}

Chat::~Chat()
{
    delete ui;
}

void Chat::loadChatHistory(std::vector<std::map<std::string, QString> > &chatHistory)
{
    QDateTime previousDateTime{};
    for(auto& messageEntity : chatHistory){
        std::string time{messageEntity["time"].toStdString()};
        time = time.substr(0, time.find("+")-4);
        auto correctTime {QDateTime::fromString(QString::fromStdString(time), "yyyy-MM-dd hh:mm")};
        bool result{messageEntity["receiver"].toStdString() != std::to_string(friendId_)};
        bool createDateWidget {false};
        if(previousDateTime.date() != correctTime.date()){
            previousDateTime = correctTime;
            createDateWidget = true;
        }
        QString hoursAndMinutes{correctTime.time().toString()};
        hoursAndMinutes.chop(3);
        MessageInfo msgInfo{messageEntity["messageGuid"],
                            messageEntity["sender"].toULongLong(),
                            messageEntity["receiver"].toULongLong(),
                            messageEntity["message"],
                            hoursAndMinutes,
                            !result};
        receiveMessage(msgInfo, correctTime, createDateWidget);
    }
}

void Chat::on_sendMsgBtn_clicked()
{
    ui->cancelEditButton->setVisible(false);
    if(ui->sendMsgBtn->text() == "Send"){
        auto currentDateTime{QDateTime::currentDateTime()};
        if(lastMessageDateTime.date() != currentDateTime.date()){
            MessagesDateWidget* dateWidget{new MessagesDateWidget{currentDateTime}};
            auto item2 {new QListWidgetItem{}};
            item2->setSizeHint(dateWidget->sizeHint());
            ui->messageList->addItem(item2);
            ui->messageList->setItemWidget(item2, dateWidget);
            lastMessageDateTime = currentDateTime;
        }
        if(!ui->msgFiled->text().isEmpty()){
            QString meesageToSend{ui->msgFiled->text()};
            processMessage(meesageToSend, true);
            for(auto& n : vectorOfMessages){
                auto item {new QListWidgetItem{}};
                item->setSizeHint(n->sizeHint());
                ui->messageList->addItem(item);
                ui->messageList->setItemWidget(item, n);
                int rowIndex = ui->messageList->row(item); // Get the row index
                messagesMap[n->getMessageInfo().messageGuid.toStdString()] = rowIndex;
                emit sendMessage(n->getMessageInfo());
            }
            ui->messageList->scrollToBottom();
            vectorOfMessages.clear();
        }
    }else{
        auto newText{ui->msgFiled->text()};
        currentlyEditingMessage_.text = newText;
        emit editMessageInDb(currentlyEditingMessage_);
        editMessageIfExists(currentlyEditingMessage_.messageGuid, newText);
        ui->sendMsgBtn->setText("Send");
    }
    ui->msgFiled->clear();
}

void Chat::receiveMessage(const MessageInfo &msgInfo, const QDateTime& sentTime, bool createDateWidget)
{
    try{
        auto item {new QListWidgetItem{}};
        MessageWidget* tmpWidget{new MessageWidget{msgInfo, mediator_}};
        if(createDateWidget || (!lastMessageDateTime.isNull() && lastMessageDateTime.date() != sentTime.date())
            || !lastMessageDateTime.isValid()){
            MessagesDateWidget* dateWidget{new MessagesDateWidget{sentTime}};
            auto item2 {new QListWidgetItem{}};
            item2->setSizeHint(dateWidget->sizeHint());
            ui->messageList->addItem(item2);
            ui->messageList->setItemWidget(item2, dateWidget);
        }

        item->setSizeHint(tmpWidget->sizeHint());
        ui->messageList->addItem(item);
        ui->messageList->setItemWidget(item, tmpWidget);
        int rowIndex = ui->messageList->row(item); // Get the row index
        messagesMap[tmpWidget->getMessageInfo().messageGuid.toStdString()] = rowIndex;
        ui->messageList->scrollToBottom();
        vectorOfMessages.clear();
        lastMessageDateTime = sentTime;
    }catch(std::exception& exc){
        qDebug()<<exc.what();
    }
}


void Chat::on_msgFiled_returnPressed()
{
    on_sendMsgBtn_clicked();
}

void Chat::processMessage(const QString &msg, bool isAuthor)
{
    QString workingString{};
    if(!hasSpaces(msg)){
        workingString = createWrap(msg);

    }else{
        workingString = msg;
    }
    if(workingString.size()>1024){
        splitIntoMessages(workingString, isAuthor);
    }else{
        createAndPushMessageWidget(workingString, isAuthor);
    }
}

int Chat::getClosestPunctuationMarkPosition(const QString &msg, bool isLeftToRight)
{
    if(isLeftToRight){
        auto iter{std::find_if(msg.rbegin(), msg.rend(), [](const QChar character){return character == '.' ||
                                                                                                     character == '!' ||
                                                                                                     character == ';' ||
                                                                                                     character == '?' ||
                                                                                                     character == ':';})};
        if(iter!=msg.rend()){
            return std::distance(msg.rbegin(), iter);
        }
    }
    auto iter{std::find_if(msg.begin(), msg.end(), [](const QChar character){return character == '.' ||
                                                                                                 character == '!' ||
                                                                                                 character == ';' ||
                                                                                                 character == '?' ||
                                                                                                 character == ':';})};
    if(iter!=msg.end()){
        return std::distance(msg.begin(), iter);
    }
    return 0;
}

void Chat::splitIntoMessages(const QString &msg, bool isAuthor)
{
    int limit {1024};
    QString textToLeft{ msg.mid(0, limit)};
    QString textToRight{ msg.mid(limit) };
    auto leftPunctuations{getClosestPunctuationMarkPosition(textToLeft, true)};
    auto rightPunctuations{getClosestPunctuationMarkPosition(textToRight, false)};
    auto toTheLeft{ limit -  leftPunctuations};
    auto toTheRight{ rightPunctuations };
    QString newString {};
    if (toTheLeft < toTheRight) {
        newString = msg.mid(0, limit - toTheLeft +1);
        createAndPushMessageWidget(newString, isAuthor);
    }
    else {
        newString = msg.mid(0, toTheRight + limit + 1);
        createAndPushMessageWidget(newString, isAuthor);
    }
    newString = msg.mid(newString.size());
    if(newString.size() > limit){
        splitIntoMessages(newString, isAuthor);
        return;
    }
    createAndPushMessageWidget(newString, isAuthor);
}

bool Chat::hasSpaces(const QString &str)
{
    auto searchResults{std::find_if(str.begin(), str.end(), [](const QChar character){return character==' ';})};
    if(searchResults!=str.end()){
        return true;
    }
    return false;
}

QString Chat::getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm time_info;
    localtime_s(&time_info, &in_time_t);
    std::stringstream ss;
    ss << std::put_time(&time_info, "%R");
    return ss.str().c_str();
}

QString Chat::createWrap(const QString &str)
{
    auto stringSize{str.size()};
    QString workingString{};
    for(int i{0}; i < stringSize; i++){
        if(i%30==0 && i!=0){
            workingString.append("\n");
        }
        workingString.append(str[i]);
    }
    return workingString;
}

std::chrono::system_clock::time_point Chat::getChronoTime(const std::string &timeStr)
{
    std::tm tm = {};
    std::stringstream ss(timeStr);
    ss >> std::get_time(&tm, "%Y %m %d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

void Chat::createAndPushMessageWidget(const QString &msg, bool isAuthor)
{
    boost::uuids::random_generator generator;
    MessageInfo msgInfo{boost::uuids::to_string(generator()).c_str(), 0, friendId_, msg, getCurrentTime(), isAuthor};
    MessageWidget* tmpWidget{new MessageWidget{msgInfo, mediator_}};
    vectorOfMessages.push_back(tmpWidget);
}

void Chat::editMessageIfExists(const QString& messageGuid, const QString& newText)
{
    auto it = messagesMap.find(messageGuid.toStdString());
    if (it != messagesMap.end()) {
        int rowIndex = it->second;
        QListWidgetItem* item = ui->messageList->item(rowIndex);
        if (item) {
            MessageWidget* widgetPtr = qobject_cast<MessageWidget*>(ui->messageList->itemWidget(item));
            widgetPtr->editMessage(newText);
        }
    }
}

std::optional<MessageInfo> Chat::getLastMessage()
{
    auto count =  ui->messageList->count() - 1;
    qDebug()<< "MSG List Count "<< count;
    QListWidgetItem* item = ui->messageList->item(count);
    MessageWidget* widgetPtr = qobject_cast<MessageWidget*>(ui->messageList->itemWidget(item));
    if(widgetPtr!=nullptr){
        return widgetPtr->getMessageInfo();
    }
    return{};
}

void Chat::onContextMenuMessageRemovalSignal(const MessageInfo & msgInfo)
{
    auto it = messagesMap.find(msgInfo.messageGuid.toStdString());
    if (it != messagesMap.end()) {
        int rowIndex = it->second;
        QListWidgetItem* item = ui->messageList->item(rowIndex);

        // Remove the QListWidgetItem from the QListWidget
        if (item) {
            MessageWidget* widgetPtr = qobject_cast<MessageWidget*>(ui->messageList->itemWidget(item));
            ui->messageList->removeItemWidget(item);
            delete item;
            delete widgetPtr;
        }

        QListWidgetItem* item2 = ui->messageList->item(ui->messageList->count() - 1);
        if(item2){
            // TODO: I need to make sure that this is MessagesDateWidget. If it is not, then do not delete it
            MessagesDateWidget* widgetPtr2 = qobject_cast<MessagesDateWidget*>(ui->messageList->itemWidget(item2));
            if(widgetPtr2){
                ui->messageList->removeItemWidget(item2);
                delete item2;
                delete widgetPtr2;
                lastMessageDateTime = QDateTime::fromMSecsSinceEpoch(0);
            }
        }

        // Remove the entry from the map
        messagesMap.erase(it);
        // Update the indices in the map
        for (auto &pair : messagesMap) {
            if (pair.second > rowIndex) {
                pair.second--;
            }
        }
    }
}

void Chat::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void Chat::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }

    QString filePath = urls.first().toLocalFile();
    if (filePath.isEmpty()) {
        return;
    }

    FileDropDialog fileDropDialog(filePath, this);
    if (fileDropDialog.exec() == QDialog::Accepted) {
        // Handle the accepted file drop
        emit sendFile(filePath.toStdString(), friendId_);
        qDebug() << "File accepted:" << filePath;
    } else {
        qDebug() << "File rejected:" << filePath;
    }
}

void Chat::onEditMessageRequested(const MessageInfo & msgInfo) {
    ui->cancelEditButton->setVisible(true);
    ui->msgFiled->setText(msgInfo.text);
    ui->sendMsgBtn->setText(" OK ");
    currentlyEditingMessage_ = msgInfo;
}

void Chat::on_cancelEditButton_clicked()
{
    ui->msgFiled->clear();
    ui->sendMsgBtn->setText("Send");
    ui->cancelEditButton->setVisible(false);
    currentlyEditingMessage_ = MessageInfo{"", 0, 0, "", "", true};
}

