#include "Contact.h"

QString &Contact::getName()
{
    return name_;
}

unsigned long long Contact::getId()
{
    return id_;
}

Contact::Contact(const QString& name, unsigned long long id): name_{name}, id_{id}
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

