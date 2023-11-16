#include "SettingsDialog.h"
#include "qpainter.h"
#include "ui_SettingsDialog.h"
#include <QPushButton>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QFileDialog>
#include <QBuffer>


SettingsDialog::SettingsDialog(QWidget *parent, const QString& userNickname) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    userNickname_{userNickname},
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
    changePasswordButton = new QPushButton("Change Password", this);
    changePasswordButton->setStyleSheet(R"(
        QPushButton {
            background-color: #4E5A65; /* A neutral, dark shade for the button */
            color: #FFFFFF; /* White text */
            font-weight: bold;
            border: 2px solid #4E5A65; /* Same as background for a flat look */
            border-radius: 5px; /* Rounded corners */
            padding: 5px 10px; /* Top/bottom padding 10px, left/right padding 15px */
            margin-bottom: 10px; /* Margin at the bottom to separate from the delete button */
        }
        QPushButton:hover {
            background-color: #63707A; /* A lighter shade for hover state */
            border-color: #63707A;
        }
        QPushButton:pressed {
            background-color: #55616A; /* A different shade for pressed state */
            border-color: #55616A;
        }
        QPushButton:disabled {
            background-color: #3A3F44; /* A darker shade for disabled state */
            color: #6E7B85; /* A lighter text color for disabled state */
            border-color: #3A3F44;
        }
    )");
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
    passwordChangeWidget = new QWidget(this);
    passwordChangeWidget->setStyleSheet(R"(
        QWidget {
            background-color: #2E2E3D;
        }
        QLabel {
            color: #FFFFFF;
            font: bold 14px;
            margin-bottom: 5px; /* Space between label and line edit */
        }
        QLineEdit {
            background-color: #4A4A5A;
            color: #FFFFFF;
            border: 1px solid #808080;
            border-radius: 4px;
            padding: 5px;
            margin-bottom: 10px; /* Space between the line edits and between line edit and button */
        }
        QPushButton {
            background-color: #4E5A65;
            color: #FFFFFF;
            font-weight: bold;
            border: 2px solid #4E5A65;
            border-radius: 5px;
            margin-bottom: 10px;
        }
        QPushButton:hover {
            background-color: #63707A;
            border-color: #63707A;
        }
        QPushButton:pressed {
            background-color: #55616A;
            border-color: #55616A;
        }
        QPushButton:disabled {
            background-color: #3A3F44;
            color: #6E7B85;
            border-color: #3A3F44;
        }
    )");
    profilePictureLabel = new ClickableLabel(this);
    profilePictureLabel->setCursor(Qt::PointingHandCursor);
    nicknameLabel = new QLabel(this);
    auto path {QCoreApplication::applicationDirPath() + "/Assets/avatar.png"};
    if(QFileInfo::exists(path)){
        QPixmap originalPixmap {path};
        customizeAvatar(originalPixmap);
    }else{
        QPixmap originalPixmap {QCoreApplication::applicationDirPath() + "/Assets/noProfileImage.jpg"};
        customizeAvatar(originalPixmap);
    }
    nicknameLabel->setText(userNickname);
    nicknameLabel->setAlignment(Qt::AlignCenter);
    nicknameLabel->setStyleSheet("QLabel { color: #FFFFFF; font: bold 14px; }"); // Example style

    newPasswordEdit = new QLineEdit(passwordChangeWidget);
    confirmPasswordEdit = new QLineEdit(passwordChangeWidget);
    submitPasswordButton = new QPushButton("Submit New Password", passwordChangeWidget);
    cancelPasswordButton = new QPushButton("Cancel", passwordChangeWidget);
    submitPasswordButton->setStyleSheet(passwordChangeWidget->styleSheet());
    cancelPasswordButton->setStyleSheet(passwordChangeWidget->styleSheet());

    // Widgets setup
    hLayout->addWidget(authLabel);
    hLayout->addWidget(toggleButton);
    profileLayout->addWidget(profilePictureLabel);
    profileLayout->addWidget(nicknameLabel);

    // Set echo mode for password line edits
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);

    // Layout for the password change widget
    QVBoxLayout *passwordLayout = new QVBoxLayout(passwordChangeWidget);
    passwordLayout->addWidget(new QLabel("New Password:", passwordChangeWidget));
    passwordLayout->addWidget(newPasswordEdit);
    passwordLayout->addWidget(new QLabel("Confirm Password:", passwordChangeWidget));
    passwordLayout->addWidget(confirmPasswordEdit);
    passwordLayout->addWidget(submitPasswordButton);
    passwordLayout->addWidget(cancelPasswordButton);

    // Initialize the stacked widget
    stackedWidget = new QStackedWidget(this);

    // Create the first page with existing widgets
    firstPageWidget = new QWidget();
    QVBoxLayout *firstPageLayout = new QVBoxLayout(firstPageWidget);
    firstPageLayout->addLayout(profileLayout);

    firstPageLayout->addLayout(hLayout);
    firstPageLayout->addWidget(changePasswordButton);
    firstPageLayout->addWidget(deleteAccountButton);

    stackedWidget->addWidget(firstPageWidget);
    stackedWidget->addWidget(codeVerificationWidget_.get());
    stackedWidget->addWidget(passwordChangeWidget);
    connect(&*codeVerificationWidget_, &CodeVerificationWidget::backButtonSignal, this, &SettingsDialog::onCodeVerificationBackButtonSignal);
    connect(&*codeVerificationWidget_, &CodeVerificationWidget::sendInputSignal, this, &SettingsDialog::onInputSendSignal);

    mainLayout->addWidget(stackedWidget);
    ui->verticalLayout_2->addLayout(mainLayout);

    // Signal/slots setup
    connect(toggleButton, &QAbstractButton::toggled, this, &SettingsDialog::handleToggleButton);
    connect(deleteAccountButton, &QPushButton::clicked, this, &SettingsDialog::onDeleteAccountClicked);
    connect(changePasswordButton, &QPushButton::clicked, this, [this] {
        stackedWidget->setCurrentWidget(passwordChangeWidget);
    });
    connect(cancelPasswordButton, &QPushButton::clicked, this, [this] {
        stackedWidget->setCurrentWidget(firstPageWidget); // Make sure firstPageWidget is a member variable if it's not already
        newPasswordEdit->clear();
        confirmPasswordEdit->clear();
    });
    connect(submitPasswordButton, &QPushButton::clicked, this, &SettingsDialog::onChangePasswordSubmitted);
    connect(profilePictureLabel, &ClickableLabel::clicked, this, &SettingsDialog::onChangeAvatarClicked);
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

void SettingsDialog::onChangePasswordSubmitted() {
    QString newPassword = newPasswordEdit->text();
    QString confirm = confirmPasswordEdit->text();

    if (newPassword.isEmpty() || confirm.isEmpty()) {
        showTemporaryPopup("Password fields cannot be empty.");
        return;
    }

    if (newPassword != confirm) {
        showTemporaryPopup("Passwords do not match.");
        return;
    }

    emit changePassword(newPassword.toStdString());

    showTemporaryPopup("Password changed successfully!");

    stackedWidget->setCurrentWidget(firstPageWidget);
    newPasswordEdit->clear();
    confirmPasswordEdit->clear();
}

void SettingsDialog::showTemporaryPopup(const QString &message) {
    QLabel *popup = new QLabel(message, this);
    popup->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    popup->setAttribute(Qt::WA_TranslucentBackground);
    popup->setStyleSheet(R"(
        QLabel {
            color: #FFFFFF;
            background-color: rgba(50, 50, 50, 220);
            border-radius: 8px;
            padding: 15px;
            font-size: 16px;
        }
    )");

    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(popup);
    effect->setBlurRadius(10);
    effect->setColor(Qt::black);
    effect->setOffset(2);
    popup->setGraphicsEffect(effect);

    popup->adjustSize();

    QPoint globalPos = this->mapToGlobal(this->rect().center());
    popup->move(globalPos.x() - popup->width() / 2, globalPos.y() - popup->height() / 2 - 200);
    popup->show();

    QTimer::singleShot(2000, popup, &QLabel::deleteLater);
}

void SettingsDialog::onChangeAvatarClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Avatar"), "", tr("Images (*.png *.jpg *.jpeg *.bmp *.gif)"));
    if (!fileName.isEmpty()) {
        QPixmap originalPixmap(fileName);
        originalPixmap.toImage().save(QCoreApplication::applicationDirPath() + "/Assets/avatar.png");
        customizeAvatar(originalPixmap);
//        QByteArray byteArray;
//        QDataStream stream(&byteArray, QIODevice::WriteOnly);
//        stream << originalPixmap;
//        emit updateAvatar(base64String.toStdString());
    }
}

void SettingsDialog::customizeAvatar(QPixmap& originalPixmap)
{
    originalPixmap = originalPixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QBitmap mask(originalPixmap.size());
    mask.fill(Qt::color0);
    QPainter painter(&mask);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(Qt::color1);
    painter.drawEllipse(0, 0, mask.width(), mask.height());
    originalPixmap.setMask(mask);
    profilePictureLabel->setPixmap(originalPixmap);
    profilePictureLabel->setAlignment(Qt::AlignCenter);
}
