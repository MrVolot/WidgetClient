#pragma once

#include <QWidget>
#include "Mediator.h"

namespace Ui {
class MessageContextMenu;
}

class MessageContextMenu : public QWidget
{
    Q_OBJECT

public:
    explicit MessageContextMenu(const MessageInfo& msgInfo, Mediator *mediator, QWidget *parent = nullptr);
    ~MessageContextMenu();
    void popup(const QPoint& pos);
    void setText(const QString& newText);
private slots:
    void on_notificationReminderButton_clicked();
    void on_deleteButton_clicked();

    void on_editButton_clicked();

private:
    Ui::MessageContextMenu *ui;
    MessageInfo messageInfo;
    Mediator *mediator_;
signals:
    void launchReminder(const MessageInfo& msgInfo);
    void deleteMessage();
    void destroyMessageWidget(const MessageInfo& msgInfo);
    void editRequested(const MessageInfo& msgInfo);
};

