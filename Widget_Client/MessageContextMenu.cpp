#include "MessageContextMenu.h"
#include "ui_MessageContextMenu.h"

MessageContextMenu::MessageContextMenu(const MessageInfo& msgInfo, Mediator *mediator, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessageContextMenu),
    messageInfo{msgInfo},
    mediator_{mediator}
{
    setWindowFlags(Qt::Popup);
    ui->setupUi(this);

    connect(this, &MessageContextMenu::launchReminder, mediator_, &Mediator::onContextMenuSignal);
    connect(this, &MessageContextMenu::destroyMessageWidget, mediator_, &Mediator::onContextMenuMessageRemovalSignal);
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
    emit launchReminder(messageInfo);
    hide();
}


void MessageContextMenu::on_deleteButton_clicked()
{
    //destroy MessageWidget object
    emit destroyMessageWidget(messageInfo);
}

