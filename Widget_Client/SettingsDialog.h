#pragma once

#include <QDialog>
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
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    void setCurrentEmail(const std::string& email);
    void showWindow();
private slots:
    void handleToggleButton(bool checked);
    void onCodeVerificationBackButtonSignal();
    void onInputSendSignal(const std::string& input);
private:
    Ui::SettingsDialog *ui;
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
signals:
    void sendEmailForVerificationSignal(const std::string& email);
    void sendVerificationCodeSignal(const std::string& code);
    void disableEmailAuthenticationSignal();
public slots:
    void retrieveCodeVerificationResult(bool result);
};


