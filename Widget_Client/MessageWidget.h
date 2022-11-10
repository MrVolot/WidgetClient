#pragma once

#include <QLabel>
#include <QWidget>

namespace Ui {
class MessageWidget;
}

class MessageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MessageWidget(const QString& text, const QString& time, QWidget *parent = nullptr);
    ~MessageWidget();

private:
    Ui::MessageWidget *ui;
};
