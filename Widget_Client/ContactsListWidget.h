#pragma once

#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>
#include "Contact.h"
#include "ContactsWidget.h"

namespace Ui {
class ContactsListWidget;
}

class ContactsListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContactsListWidget(QWidget *parent = nullptr);
    ~ContactsListWidget();
    void setCurrentFriendId(unsigned long long id);
    unsigned long long getCurrentFriendId();
    QListWidget* getContactsWidget();
    void showTemporaryContactsWidget(const std::vector<Contact>& contacts);
    void showMainContactsWidget();
    void addContacts(std::vector<Contact> friendList, bool isMain = true);
    std::optional<std::pair<QListWidgetItem*, ContactsWidget*>> findFriendById(unsigned long long id);
private:
    Ui::ContactsListWidget *ui;
    std::unique_ptr<QListWidget> mainContactsWidget;
    std::unique_ptr<QListWidget> tempContactsWidget;
    bool isMainWidgetOn;
    unsigned long long currentFriend;

    void setCurrentChatAndLoadChatInfo(QListWidgetItem *item, Contact& contact);
private slots:
    void on_contactsWidget_itemClicked(QListWidgetItem *item);
    void on_tempContactsWidget_itemClicked(QListWidgetItem *item);
public slots:
    bool setAndOpenChat(unsigned long long id);
signals:
    void loadChatInfo(Contact& contact);
    void cleanSearchLine();
};

