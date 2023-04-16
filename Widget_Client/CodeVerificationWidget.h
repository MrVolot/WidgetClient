#pragma once

#include <QKeyEvent>
#include <QWidget>

namespace Ui {
class CodeVerificationWidget;
}

class CodeVerificationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CodeVerificationWidget(QWidget *parent = nullptr);
    ~CodeVerificationWidget();
    void setLabelText(const std::string& text);
    void cleanInputField();
private:
    Ui::CodeVerificationWidget *ui;
private slots:
    void on_verCodeInput_returnPressed();
    void on_backButton_clicked();
public slots:
    void onCleanAndShowError(const std::string& errorMessage);
signals:
    void sendInputSignal(const std::string& verCode);
    void backButtonSignal();
protected:
    void keyPressEvent(QKeyEvent *event);
};

