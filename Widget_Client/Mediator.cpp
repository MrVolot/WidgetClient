
#include "Mediator.h"

Mediator::Mediator(QObject *parent): QObject(parent)
{

}

void Mediator::onContextMenuSignal(const MessageInfo & msgInfo, size_t mins)
{
    emit contextMenuSignal(msgInfo, mins);
}

void Mediator::onContextMenuMessageRemovalSignal(const MessageInfo & msgInfo)
{
    MessageInfo msgInfoCopy{msgInfo};
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
