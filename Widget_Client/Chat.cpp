#include "Chat.h"
#include "ui_Chat.h"
#include "MessagesDateWidget.h"
#include <sstream>
#include <iomanip>
#include <iterator>
#include <QDate>

Chat::Chat(const QString& nameArg, unsigned long long idArg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Chat),
    name{nameArg},
    id{idArg}
{
    ui->setupUi(this);
    ui->receiverName->setText(name);
    ui->messageList->setStyleSheet("QListWidget::item:hover {background: transparent;} "
                                   "QListWidget::item:selected:!active {background: transparent;} ");
    ui->messageList->setFocusPolicy(Qt::NoFocus);
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
        bool result{messageEntity["receiver"].toStdString() != std::to_string(id)};
        auto currentDateTime{QDateTime::currentDateTime()};
        bool createDateWidget {false};
        if(previousDateTime.date() != correctTime.date()){
            previousDateTime = correctTime;
            createDateWidget = true;
        }
        receiveMessage(messageEntity["message"], correctTime, createDateWidget, !result);
    }
}

void Chat::on_sendMsgBtn_clicked()
{
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
            emit sendMessage(n->getText(), id);
        }
        ui->messageList->scrollToBottom();
        ui->msgFiled->clear();
        vectorOfMessages.clear();
    }
}

void Chat::receiveMessage(const QString &msg, const QDateTime& sentTime, bool createDateWidget, bool isAuthor)
{
    try{
    auto item {new QListWidgetItem{}};
    auto correctTime{sentTime.time().toString()};
    correctTime.chop(3);
    MessageWidget* tmpWidget{new MessageWidget{msg, correctTime, isAuthor}};

    if(createDateWidget || (!lastMessageDateTime.isNull() && lastMessageDateTime.date() != sentTime.date())){
        MessagesDateWidget* dateWidget{new MessagesDateWidget{sentTime}};
        auto item2 {new QListWidgetItem{}};
        item2->setSizeHint(dateWidget->sizeHint());
        ui->messageList->addItem(item2);
        ui->messageList->setItemWidget(item2, dateWidget);
    }

    item->setSizeHint(tmpWidget->sizeHint());
    ui->messageList->addItem(item);
    ui->messageList->setItemWidget(item, tmpWidget);
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
        vectorOfMessages.push_back(new MessageWidget{workingString, getCurrentTime(), isAuthor});
    }
}

int Chat::getClosestPunctuationMarkPosition(const QString &msg, bool isLeftToRight)
{
    QString punctuationMarks{"."};//!;?:
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
        vectorOfMessages.push_back(new MessageWidget{newString, getCurrentTime(), isAuthor});
    }
    else {
        newString = msg.mid(0, toTheRight + limit + 1);
        vectorOfMessages.push_back(new MessageWidget{newString, getCurrentTime(), isAuthor});
    }
    newString = msg.mid(newString.size());
    if(newString.size() > limit){
        splitIntoMessages(newString, isAuthor);
        return;
    }
    vectorOfMessages.push_back(new MessageWidget{newString, getCurrentTime(), isAuthor});
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
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%R");
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

