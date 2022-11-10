/********************************************************************************
** Form generated from reading UI file 'MessageWidget.ui'
**
** Created by: Qt User Interface Compiler version 6.3.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MESSAGEWIDGET_H
#define UI_MESSAGEWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MessageWidget
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QLabel *message;
    QLabel *time;

    void setupUi(QWidget *MessageWidget)
    {
        if (MessageWidget->objectName().isEmpty())
            MessageWidget->setObjectName(QString::fromUtf8("MessageWidget"));
        MessageWidget->setWindowModality(Qt::NonModal);
        MessageWidget->resize(300, 300);
        MessageWidget->setMaximumSize(QSize(300, 16777215));
        verticalLayout_2 = new QVBoxLayout(MessageWidget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        message = new QLabel(MessageWidget);
        message->setObjectName(QString::fromUtf8("message"));
        message->setMaximumSize(QSize(300, 16777215));
        message->setWordWrap(true);

        verticalLayout->addWidget(message);

        time = new QLabel(MessageWidget);
        time->setObjectName(QString::fromUtf8("time"));
        time->setAlignment(Qt::AlignBottom|Qt::AlignRight|Qt::AlignTrailing);

        verticalLayout->addWidget(time);


        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(MessageWidget);

        QMetaObject::connectSlotsByName(MessageWidget);
    } // setupUi

    void retranslateUi(QWidget *MessageWidget)
    {
        MessageWidget->setWindowTitle(QCoreApplication::translate("MessageWidget", "Form", nullptr));
        message->setText(QCoreApplication::translate("MessageWidget", "TextLabel", nullptr));
        time->setText(QCoreApplication::translate("MessageWidget", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MessageWidget: public Ui_MessageWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MESSAGEWIDGET_H
