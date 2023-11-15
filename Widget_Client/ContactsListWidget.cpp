#include "ContactsListWidget.h"
#include "ui_ContactsListWidget.h"

ContactsListWidget::ContactsListWidget(Mediator *mediator, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContactsListWidget),
    mediator_{mediator},
    isMainWidgetOn{true},
    currentFriend{0}
{
    ui->setupUi(this);
    mainContactsWidget.reset(new QListWidget{this});
    mainContactsWidget->setStyleSheet("");
    ui->verticalLayout_2->addWidget(mainContactsWidget.get());
    connect(mainContactsWidget.get(), &QListWidget::itemClicked, this, &ContactsListWidget::on_contactsWidget_itemClicked);
}

ContactsListWidget::~ContactsListWidget()
{
    delete ui;
}

void ContactsListWidget::setCurrentFriendId(unsigned long long id)
{
    currentFriend = id;
}

unsigned long long ContactsListWidget::getCurrentFriendId()
{
    return currentFriend;
}

QListWidget* ContactsListWidget::getContactsWidget()
{
    return tempContactsWidget!=nullptr ? tempContactsWidget.get() : mainContactsWidget.get();
}

void ContactsListWidget::on_contactsWidget_itemClicked(QListWidgetItem *item)
{
    auto contact{dynamic_cast<ContactsWidget*>(mainContactsWidget->itemWidget(item))->getContact()};
    if(contact.getId() == currentFriend){
        return;
    }
    emit loadChatInfo(contact);
}

void ContactsListWidget::showTemporaryContactsWidget(const std::vector<Contact>& contacts)
{
    if(isMainWidgetOn){
        // Create a temporary widget for the possible contacts
        isMainWidgetOn = false;
        tempContactsWidget.reset(new QListWidget{this});
        addContacts(contacts, false);
        ui->verticalLayout_2->replaceWidget(mainContactsWidget.get(), tempContactsWidget.get());
        connect(tempContactsWidget.get(), &QListWidget::itemClicked, this, &ContactsListWidget::on_tempContactsWidget_itemClicked);
    }else {
        // Update the tempContactsWidget with the new contacts
        tempContactsWidget->clear();
        addContacts(contacts, false);
    }
    ui->verticalLayout_2->update();
}

void ContactsListWidget::showMainContactsWidget()
{
    if(!isMainWidgetOn){
        if(tempContactsWidget!=nullptr){
            disconnect(tempContactsWidget.get(), &QListWidget::itemClicked, this, &ContactsListWidget::on_tempContactsWidget_itemClicked);
            tempContactsWidget->setVisible(false);
            ui->verticalLayout_2->replaceWidget(tempContactsWidget.get(), mainContactsWidget.get());
            isMainWidgetOn = true;
        }
    }
    ui->verticalLayout_2->update();
}

void ContactsListWidget::addContacts(std::vector<Contact> friendList, bool isMain)
{
    for(auto& contactItem : friendList){
        auto item {new QListWidgetItem{}};
        ContactsWidget* contactWidget{createContact(contactItem)};
        item->setSizeHint(contactWidget->sizeHint());
        if(isMain){
            mainContactsWidget->addItem(item);
            mainContactsWidget->setItemWidget(item, contactWidget);
        }else{
            tempContactsWidget->addItem(item);
            tempContactsWidget->setItemWidget(item, contactWidget);
            contactWidget->hideDeleteChatButton();
        }
    }
}

std::pair<QListWidgetItem*, ContactsWidget*> ContactsListWidget::constructContact(Contact contact)
{
    auto item {new QListWidgetItem{}};
    ContactsWidget* contactWidget{createContact(contact)};
    item->setSizeHint(contactWidget->sizeHint());
    mainContactsWidget->addItem(item);
    mainContactsWidget->setItemWidget(item, contactWidget);
    return std::make_pair(item, contactWidget);
}

void ContactsListWidget::on_tempContactsWidget_itemClicked(QListWidgetItem *item)
{
    auto contactInfo{dynamic_cast<ContactsWidget*>(tempContactsWidget->itemWidget(item))->getContact()};
    if(!setAndOpenChat(contactInfo.getId())){
        ContactsWidget* contact{createContact(contactInfo)};
        auto newItem = new QListWidgetItem{};
        newItem->setSizeHint(contact->sizeHint());
        mainContactsWidget->addItem(newItem);
        mainContactsWidget->setItemWidget(newItem, contact);
        setCurrentChatAndLoadChatInfo(newItem, contact->getContact());
    }
    emit cleanSearchLine();
}

bool ContactsListWidget::setAndOpenChat(unsigned long long id)
{
    auto foundFriend{findFriendById(id)};
    if(foundFriend.has_value()){
        setCurrentChatAndLoadChatInfo(foundFriend.value().first, foundFriend.value().second->getContact());
        return true;
    }
    return false;
}

std::optional<std::pair<QListWidgetItem*, ContactsWidget*>> ContactsListWidget::findFriendById(unsigned long long id)
{
    for (int i = 0; i < mainContactsWidget->count(); ++i) {
        QListWidgetItem* item =  mainContactsWidget->item(i);
        ContactsWidget* contactsWidget = qobject_cast<ContactsWidget*>( mainContactsWidget->itemWidget(item));
        if (contactsWidget && contactsWidget->getContact().getId() == id) {
            return std::make_pair(item, contactsWidget);
        }
    }
    return {};
}

void ContactsListWidget::removeChat(unsigned long long id)
{
    for (int i = 0; i < mainContactsWidget->count(); ++i) {
        QListWidgetItem* item = mainContactsWidget->item(i);
        ContactsWidget* contactsWidget = dynamic_cast<ContactsWidget*>(mainContactsWidget->itemWidget(item));
        if (contactsWidget && contactsWidget->getContact().getId() == id) {
            delete mainContactsWidget->takeItem(i);
            delete contactsWidget;
            break;
        }
    }
}

void ContactsListWidget::setLastMessage(unsigned long long chatId, const QString &message, bool isAuthor)
{
    auto foundFriend {findFriendById(chatId)};
    if(foundFriend.has_value()){
        foundFriend.value().second->setLastMessage(isAuthor, message);
    }
}

void ContactsListWidget::setCurrentChatAndLoadChatInfo(QListWidgetItem *item, Contact& contact)
{
    mainContactsWidget->setCurrentItem(item);
    emit loadChatInfo(contact);
}

ContactsWidget *ContactsListWidget::createContact(Contact contact)
{
    ContactsWidget* contactWidget{new ContactsWidget{contact, mediator_}};
    connect(contactWidget, &ContactsWidget::deleteChatSignal, &*mediator_, &Mediator::onDeleteChatSignal);
    return contactWidget;
}
