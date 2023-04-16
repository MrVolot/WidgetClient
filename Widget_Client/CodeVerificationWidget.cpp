#include "CodeVerificationWidget.h"
#include "ui_CodeVerificationWidget.h"

CodeVerificationWidget::CodeVerificationWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CodeVerificationWidget)
{
    ui->setupUi(this);
    ui->verText->setWordWrap(true);
    ui->backButton->setDefault(false);
}

CodeVerificationWidget::~CodeVerificationWidget()
{
    delete ui;
}

void CodeVerificationWidget::setLabelText(const std::string &text)
{
    ui->verText->setText(text.c_str());
}

void CodeVerificationWidget::cleanInputField()
{
    ui->verCodeInput->clear();
}

void CodeVerificationWidget::on_verCodeInput_returnPressed()
{
    emit sendInputSignal(ui->verCodeInput->text().toStdString());
}

void CodeVerificationWidget::on_backButton_clicked()
{
    emit backButtonSignal();
}

void CodeVerificationWidget::onCleanAndShowError(const std::string &errorMessage)
{
    ui->verCodeInput->clear();
    ui->verCodeInput->setPlaceholderText(errorMessage.c_str());
}

void CodeVerificationWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        if (ui->verCodeInput->hasFocus())
        {
            // Do nothing, as returnPressed signal will call on_verCodeInput_returnPressed
        }
        else if (ui->backButton->hasFocus())
        {
            emit backButtonSignal();
        }
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}
