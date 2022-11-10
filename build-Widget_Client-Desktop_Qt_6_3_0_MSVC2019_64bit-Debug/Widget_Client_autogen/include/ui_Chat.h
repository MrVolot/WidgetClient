/********************************************************************************
** Form generated from reading UI file 'Chat.ui'
**
** Created by: Qt User Interface Compiler version 6.3.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHAT_H
#define UI_CHAT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Chat
{
public:
    QVBoxLayout *verticalLayout_3;
    QVBoxLayout *verticalLayout_2;
    QLabel *receiverName;
    QListWidget *messageList;
    QHBoxLayout *horizontalLayout;
    QLineEdit *msgFiled;
    QPushButton *sendMsgBtn;

    void setupUi(QWidget *Chat)
    {
        if (Chat->objectName().isEmpty())
            Chat->setObjectName(QString::fromUtf8("Chat"));
        Chat->resize(543, 392);
        Chat->setMinimumSize(QSize(200, 0));
        verticalLayout_3 = new QVBoxLayout(Chat);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        receiverName = new QLabel(Chat);
        receiverName->setObjectName(QString::fromUtf8("receiverName"));

        verticalLayout_2->addWidget(receiverName);

        messageList = new QListWidget(Chat);
        messageList->setObjectName(QString::fromUtf8("messageList"));

        verticalLayout_2->addWidget(messageList);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        msgFiled = new QLineEdit(Chat);
        msgFiled->setObjectName(QString::fromUtf8("msgFiled"));

        horizontalLayout->addWidget(msgFiled);

        sendMsgBtn = new QPushButton(Chat);
        sendMsgBtn->setObjectName(QString::fromUtf8("sendMsgBtn"));

        horizontalLayout->addWidget(sendMsgBtn);


        verticalLayout_2->addLayout(horizontalLayout);


        verticalLayout_3->addLayout(verticalLayout_2);


        retranslateUi(Chat);

        QMetaObject::connectSlotsByName(Chat);
    } // setupUi

    void retranslateUi(QWidget *Chat)
    {
        Chat->setWindowTitle(QCoreApplication::translate("Chat", "Form", nullptr));
        receiverName->setText(QCoreApplication::translate("Chat", "TextLabel", nullptr));
        sendMsgBtn->setText(QCoreApplication::translate("Chat", "Send", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Chat: public Ui_Chat {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHAT_H
