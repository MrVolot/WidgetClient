
#include "Mediator.h"

Mediator::Mediator(QObject *parent): QObject(parent)
{

}

void Mediator::onContextMenuSignal(const MessageInfo & msgInfo)
{
    emit contextMenuSignal(msgInfo);
}

void Mediator::onContextMenuMessageRemovalSignal(const MessageInfo & msgInfo)
{
    auto msgInfoCopy{msgInfo};
    emit contextMenuMessageRemovalSignal(msgInfoCopy);
    emit contextMenuMessageRemovalFromDbSignal(msgInfoCopy);
}

void Mediator::onEditMessage(const MessageInfo & msgInfo)
{
    emit editMessageSignal(msgInfo);
}

void Mediator::onDeleteChatSignal(unsigned long long id)
{
    emit deleteChat(id);
}
