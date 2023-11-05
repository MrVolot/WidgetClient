
#pragma once


#include "MessageInfo.h"
#include <QObject>


class Mediator: public QObject
{
    Q_OBJECT
public:
    Mediator(QObject *parent = nullptr);
signals:
    void contextMenuSignal(const MessageInfo &msgInfo);
    void contextMenuMessageRemovalSignal(const MessageInfo & msgInfo);
    void contextMenuMessageRemovalFromDbSignal(const MessageInfo & msgInfo);
    void editMessageSignal(const MessageInfo & msgInfo);
    //void removeMessageWidgetFromChat(const QString &guid);
public slots:
    void onContextMenuSignal(const MessageInfo &msgInfo);
    void onContextMenuMessageRemovalSignal(const MessageInfo & msgInfo);
    void onEditMessage(const MessageInfo & msgInfo);
};

