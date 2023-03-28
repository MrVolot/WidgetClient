#include "MessageContextMenu.h"
#include "ui_MessageContextMenu.h"

MessageContextMenu::MessageContextMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessageContextMenu)
{
    setWindowFlags(Qt::Popup);
    ui->setupUi(this);
}

MessageContextMenu::~MessageContextMenu()
{
    delete ui;
}

void MessageContextMenu::popup(const QPoint& pos)
{
    // Get global cursor position
    QPoint globalPos = QCursor::pos();

    // Map position to local coordinates
    QPoint localPos = parentWidget()->mapFromGlobal(globalPos);

    // Set position and show the widget
    move(localPos);
    show();
}

void MessageContextMenu::on_notificationReminderButton_clicked()
{
    emit launchReminder();
    hide();
}

