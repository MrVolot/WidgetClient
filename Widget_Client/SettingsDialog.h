#pragma once

#include <ClickableLabel.h>
#include <QDialog>
#include <QLineEdit>
#include <QStackedWidget>
#include "AnimatedToggleButton.h"
#include "CodeVerificationWidget.h"
#include "qboxlayout.h"
#include "qlabel.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr, bool isGuestAccount = false, const QString& userNickname = "");
    ~SettingsDialog();

    void setCurrentEmail(const std::string& email);
    void showWindow();
private slots:
    void handleToggleButton(bool checked);
    void onCodeVerificationBackButtonSignal();
    void onInputSendSignal(const std::string& input);
    void onDeleteAccountClicked();
    void onChangePasswordSubmitted();
    void onChangeAvatarClicked();
private:
    Ui::SettingsDialog *ui;
    bool isGuestAccount_;
    QString userNickname_;
    QStackedWidget *stackedWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *hLayout;
    QVBoxLayout *profileLayout;
    AnimatedToggleButton *toggleButton;
    QLabel *authLabel;
    QLabel *profilePlaceholder;
    std::string email_;
    std::unique_ptr<CodeVerificationWidget> codeVerificationWidget_;
    bool emailVarified_;
    bool isCodeVerificationStage_;
    QWidget *firstPageWidget;
    QPushButton *changePasswordButton;
    QPushButton* deleteAccountButton;
    QLineEdit *newPasswordEdit;
    QLineEdit *confirmPasswordEdit;
    QPushButton *submitPasswordButton;
    QPushButton *cancelPasswordButton;
    QWidget *passwordChangeWidget;
    ClickableLabel *profilePictureLabel;
    QLabel *nicknameLabel;

    void showTemporaryPopup(const QString &message);
    void customizeAvatar(QPixmap& originalPixmap);
    QString compressAndEncode(const QString& imagePath);
signals:
    void sendEmailForVerificationSignal(const std::string& email);
    void sendVerificationCodeSignal(const std::string& code);
    void disableEmailAuthenticationSignal();
    void deleteAccountSignal();
    void changePassword(const std::string& newPassword);
    void updateAvatar(const std::string& photoStream);
public slots:
    void retrieveCodeVerificationResult(bool result);
};


