/********************************************************************************
** Form generated from reading UI file 'RegisterDialog.ui'
**
** Created by: Qt User Interface Compiler version 6.3.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REGISTERDIALOG_H
#define UI_REGISTERDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RegisterDialog
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout_2;
    QLineEdit *Login;
    QLineEdit *Password;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *LoginButton;
    QPushButton *RegisterButton;

    void setupUi(QDialog *RegisterDialog)
    {
        if (RegisterDialog->objectName().isEmpty())
            RegisterDialog->setObjectName(QString::fromUtf8("RegisterDialog"));
        RegisterDialog->resize(290, 90);
        RegisterDialog->setMinimumSize(QSize(290, 90));
        RegisterDialog->setMaximumSize(QSize(290, 90));
        verticalLayoutWidget = new QWidget(RegisterDialog);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(0, 0, 291, 88));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        Login = new QLineEdit(verticalLayoutWidget);
        Login->setObjectName(QString::fromUtf8("Login"));
        Login->setStyleSheet(QString::fromUtf8(""));

        verticalLayout_2->addWidget(Login);

        Password = new QLineEdit(verticalLayoutWidget);
        Password->setObjectName(QString::fromUtf8("Password"));
        Password->setStyleSheet(QString::fromUtf8(""));
        Password->setEchoMode(QLineEdit::Password);

        verticalLayout_2->addWidget(Password);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        LoginButton = new QPushButton(verticalLayoutWidget);
        LoginButton->setObjectName(QString::fromUtf8("LoginButton"));

        horizontalLayout_2->addWidget(LoginButton);

        RegisterButton = new QPushButton(verticalLayoutWidget);
        RegisterButton->setObjectName(QString::fromUtf8("RegisterButton"));

        horizontalLayout_2->addWidget(RegisterButton);


        verticalLayout_2->addLayout(horizontalLayout_2);


        retranslateUi(RegisterDialog);

        QMetaObject::connectSlotsByName(RegisterDialog);
    } // setupUi

    void retranslateUi(QDialog *RegisterDialog)
    {
        RegisterDialog->setWindowTitle(QCoreApplication::translate("RegisterDialog", "Sign in", nullptr));
        Login->setPlaceholderText(QCoreApplication::translate("RegisterDialog", "Login", nullptr));
        Password->setPlaceholderText(QCoreApplication::translate("RegisterDialog", "Password", nullptr));
        LoginButton->setText(QCoreApplication::translate("RegisterDialog", "Login", nullptr));
        RegisterButton->setText(QCoreApplication::translate("RegisterDialog", "Register", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RegisterDialog: public Ui_RegisterDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REGISTERDIALOG_H
