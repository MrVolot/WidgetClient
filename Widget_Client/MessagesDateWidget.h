#pragma once

#include <QWidget>
#include <QDateTime>

namespace Ui {
class MessagesDateWidget;
}

class MessagesDateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MessagesDateWidget(QDateTime dateTime, QWidget *parent = nullptr);
    ~MessagesDateWidget();

private:
    Ui::MessagesDateWidget *ui;
};

