#pragma once

#include <QWidget>

namespace Ui {
class MessageContextMenu;
}

class MessageContextMenu : public QWidget
{
    Q_OBJECT

public:
    explicit MessageContextMenu(QWidget *parent = nullptr);
    ~MessageContextMenu();
    void popup(const QPoint& pos);
private slots:
    void on_notificationReminderButton_clicked();

private:
    Ui::MessageContextMenu *ui;
signals:
    void launchReminder();
};

