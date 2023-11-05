#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include <QPushButton>
#include <QMessageBox>


SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    emailVarified_{false},
    isCodeVerificationStage_{false}
{
    ui->setupUi(this);
    codeVerificationWidget_.reset(new CodeVerificationWidget{this});

    setStyleSheet("QDialog { background-color: #2E2E3D; }"
                  "QLabel { color: #FFFFFF; font: bold 14px; }"
                  "QPushButton { background-color: #4A4A5A; color: #FFFFFF; font: bold 14px; border: 1px solid #808080; border-radius: 4px; }"
                  "QPushButton:checked { background-color: #6A6A8A; }");

    // Memory allocation
    mainLayout = new QVBoxLayout();
    hLayout = new QHBoxLayout();
    profileLayout = new QVBoxLayout();
    authLabel = new QLabel("Authentication", this);
    toggleButton = new AnimatedToggleButton(this);
    profilePlaceholder = new QLabel("Profile information here", this);
    deleteAccountButton = new QPushButton("Delete Account", this);
    deleteAccountButton->setStyleSheet(R"(
        QPushButton {
            background-color: #D32F2F;
            color: white;
            font-weight: bold;
            border: none;
            padding: 5px 10px;
            border-radius: 5px;
        }
        QPushButton:hover {
            background-color: #E57373;
        }
        QPushButton:pressed {
            background-color: #C62828;
        }
    )");

    // Widgets setup
    hLayout->addWidget(authLabel);
    hLayout->addWidget(toggleButton);
    profileLayout->addWidget(profilePlaceholder);

    // Initialize the stacked widget
    stackedWidget = new QStackedWidget(this);

    // Create the first page with existing widgets
    QWidget *firstPageWidget = new QWidget();
    QVBoxLayout *firstPageLayout = new QVBoxLayout(firstPageWidget);
    firstPageLayout->addLayout(profileLayout);
    firstPageLayout->addLayout(hLayout);

    // Add the first page to the stacked widget
    stackedWidget->addWidget(firstPageWidget);
    stackedWidget->addWidget(codeVerificationWidget_.get());
    connect(&*codeVerificationWidget_, &CodeVerificationWidget::backButtonSignal, this, &SettingsDialog::onCodeVerificationBackButtonSignal);
    connect(&*codeVerificationWidget_, &CodeVerificationWidget::sendInputSignal, this, &SettingsDialog::onInputSendSignal);

    // Add the stacked widget to the main layout
    mainLayout->addWidget(stackedWidget);
    mainLayout->addWidget(deleteAccountButton);

    // Layouts setup
    ui->verticalLayout_2->addLayout(mainLayout);

    // Signal/slots setup
    connect(toggleButton, &QAbstractButton::toggled, this, &SettingsDialog::handleToggleButton);
    connect(deleteAccountButton, &QPushButton::clicked, this, &SettingsDialog::onDeleteAccountClicked);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::setCurrentEmail(const std::string &email)
{
    email_ = email;
    if(email!="null"){
        emailVarified_ = true;
    }
}

void SettingsDialog::showWindow()
{
    toggleButton->setToggleState(emailVarified_);
    stackedWidget->setCurrentIndex(0);
    exec();
}

void SettingsDialog::handleToggleButton(bool checked)
{
    if (checked) {
            stackedWidget->setCurrentIndex(1);
            codeVerificationWidget_->setLabelText("Please, enter your email, in order to enable authentication");
    } else {
        emit disableEmailAuthenticationSignal();
        emailVarified_ = false;
    }
}

void SettingsDialog::onCodeVerificationBackButtonSignal()
{
    isCodeVerificationStage_ = false;
    toggleButton->setToggleState(false);
    stackedWidget->setCurrentIndex(0);
}

void SettingsDialog::onInputSendSignal(const std::string &input)
{
    if(!isCodeVerificationStage_){
        isCodeVerificationStage_ = true;
        email_ = input;
        emit sendEmailForVerificationSignal(input);
        codeVerificationWidget_->setLabelText("Please, enter the verification code that was sent to the email, in order to enable authentication");
        codeVerificationWidget_->cleanInputField();
    }else{
        emit sendVerificationCodeSignal(input);
        codeVerificationWidget_->cleanInputField();
    }
}

void SettingsDialog::retrieveCodeVerificationResult(bool result)
{
    if(result == true){
        emailVarified_ = true;
        toggleButton->setToggleState(true);
        stackedWidget->setCurrentIndex(0);
    }else{
        codeVerificationWidget_->setLabelText("Incorrect code. Try again!");
        codeVerificationWidget_->cleanInputField();
    }
}

void SettingsDialog::onDeleteAccountClicked() {
    QMessageBox messageBox;
    messageBox.setWindowTitle("Confirm Deletion");
    messageBox.setText("Are you sure you want to delete your account?");
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    messageBox.setDefaultButton(QMessageBox::No);
    messageBox.setIcon(QMessageBox::Warning);

    // Apply a stylesheet to QMessageBox for a dark grey theme
    messageBox.setStyleSheet(R"(
        QMessageBox {
            background-color: #333333; /* Dark grey background */
            color: #DDDDDD; /* Light grey text for readability */
        }
        QLabel {
            color: #DDDDDD;
        }
        QPushButton {
            background-color: #555555; /* Medium grey for buttons */
            color: #FFFFFF;
            border: none;
            border-radius: 4px;
            padding: 6px 20px;
            margin: 4px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #6E6E6E; /* Lighter grey for hover state */
        }
        QPushButton:pressed {
            background-color: #4D4D4D; /* Slightly darker grey for pressed state */
        }
    )");



    int result = messageBox.exec();
    if (result == QMessageBox::Yes) {
        emit deleteAccountSignal();
    }
}
