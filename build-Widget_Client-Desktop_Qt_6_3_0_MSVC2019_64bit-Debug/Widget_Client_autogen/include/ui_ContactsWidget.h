/********************************************************************************
** Form generated from reading UI file 'ContactsWidget.ui'
**
** Created by: Qt User Interface Compiler version 6.3.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONTACTSWIDGET_H
#define UI_CONTACTSWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ContactsWidget
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QLabel *userNickname;

    void setupUi(QWidget *ContactsWidget)
    {
        if (ContactsWidget->objectName().isEmpty())
            ContactsWidget->setObjectName(QString::fromUtf8("ContactsWidget"));
        ContactsWidget->resize(400, 300);
        verticalLayout_2 = new QVBoxLayout(ContactsWidget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        userNickname = new QLabel(ContactsWidget);
        userNickname->setObjectName(QString::fromUtf8("userNickname"));
        userNickname->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(userNickname);


        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(ContactsWidget);

        QMetaObject::connectSlotsByName(ContactsWidget);
    } // setupUi

    void retranslateUi(QWidget *ContactsWidget)
    {
        ContactsWidget->setWindowTitle(QCoreApplication::translate("ContactsWidget", "Form", nullptr));
        userNickname->setText(QCoreApplication::translate("ContactsWidget", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ContactsWidget: public Ui_ContactsWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONTACTSWIDGET_H
