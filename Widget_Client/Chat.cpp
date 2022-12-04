#include "Chat.h"
#include "ui_Chat.h"
#include <sstream>
#include <iomanip>
#include <iterator>

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

void Chat::on_sendMsgBtn_clicked()
{
    if(!ui->msgFiled->text().isEmpty()){
        QString meesageToSend{ui->msgFiled->text()};
        receiveMessage(meesageToSend, 0);
        ui->msgFiled->clear();
        emit sendMessage(meesageToSend, id);
    }
}

void Chat::receiveMessage(const QString &msg, unsigned long long idArg, bool isAuthor)
{
    processMessage(msg, isAuthor);
    for(auto& n : vectorOfMessages){
        auto item {new QListWidgetItem{}};
        item->setSizeHint(n->sizeHint());
        ui->messageList->addItem(item);
        ui->messageList->setItemWidget(item, n);
    }
    ui->messageList->scrollToBottom();
    vectorOfMessages.clear();
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
    qDebug()<< toTheLeft << "" << toTheRight;
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
        if(i%30==0){
            workingString.append("\n");
        }
        workingString.append(str[i]);
    }
    return workingString;
}

