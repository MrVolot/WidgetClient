#include "Contact.h"

QString &Contact::getName()
{
    return name_;
}

unsigned long long Contact::getId()
{
    return id_;
}

std::pair<unsigned long long, const QString&> Contact::getLastMessageInfo()
{
    return lastMessageInfo_;
}

std::vector<unsigned char> &Contact::getSharedKey()
{
    return sharedKey_;
}

Contact::Contact(const QString& name, unsigned long long id, std::pair<unsigned long long, QString> lastMessageInfo, std::vector<unsigned char> sharedKey):
    name_{name},
    id_{id},
    lastMessageInfo_{lastMessageInfo},
    sharedKey_{sharedKey}
{

}

bool Contact::operator==(const Contact& rhs){
    return id_==rhs.id_;
}

bool Contact::operator>(const Contact& rhs){
    return id_>rhs.id_;
}

bool Contact::operator<(const Contact& rhs){
    return id_<rhs.id_;
}

bool Contact::operator!=(const Contact& rhs){
    return id_!=rhs.id_;
}

