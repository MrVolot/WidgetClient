#pragma once

#include <QLabel>
#include <QWidget>
#include "MessageContextMenu.h"

namespace Ui {
class MessageWidget;
}

class MessageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MessageWidget(const QString& text, const QString& time, bool isAuthor, QWidget *parent = nullptr);
    void setText(const QString& text, const QString& time,bool isAuthor);
    QString getText();
    ~MessageWidget();
private:
    Ui::MessageWidget *ui;
    std::unique_ptr<MessageContextMenu> contextMenu;
protected:
    void mousePressEvent(QMouseEvent* event) override;
signals:
    void proceedMessageReminder(const QString &msg);
public slots:
    void launchReminder();
};

