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
    explicit MessageWidget(const QString& text, const QString& time, bool isAuthor, QWidget *parent = nullptr);
    void setTextAllignment();
    void setText(const QString& text, const QString& time,bool isAuthor);
    ~MessageWidget();
private:
    Ui::MessageWidget *ui;
};

