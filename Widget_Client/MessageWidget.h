#pragma once

#include "MessageInfo.h"
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
    explicit MessageWidget(const MessageInfo& msgInfo, Mediator *mediator, QWidget *parent = nullptr);
    void setText(const QString& text, const QString& time,bool isAuthor);
    const MessageInfo& getMessageInfo();
    ~MessageWidget();
private:
    Ui::MessageWidget *ui;
    std::unique_ptr<MessageContextMenu> contextMenu;
    MessageInfo msgInfo_;
    Mediator *mediator_;
protected:
    void mousePressEvent(QMouseEvent* event) override;
};

