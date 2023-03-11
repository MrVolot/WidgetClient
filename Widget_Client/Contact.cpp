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

Contact::Contact(const QString& name, unsigned long long id, std::pair<unsigned long long, QString> lastMessageInfo): name_{name}, id_{id}, lastMessageInfo_{lastMessageInfo}
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

