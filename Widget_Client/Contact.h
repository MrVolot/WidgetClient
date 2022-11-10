#pragma once
#include <qstring.h>
#include <string>

class Contact
{
    QString name_;
    unsigned long long id_;
public:
    Contact(const QString& name, unsigned long long id);
    QString& getName();
    unsigned long long getId();

    bool operator==(const Contact& rhs);
    bool operator>(const Contact& rhs);
    bool operator<(const Contact& rhs);
    bool operator!=(const Contact& rhs);
};
