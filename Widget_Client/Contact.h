#pragma once
#include <qstring.h>
#include <string>

class Contact
{
    QString name_;
    long long id_;
public:
    Contact(const QString& name, long long id);
    QString& getName();
    long long getId();

    bool operator==(const Contact& rhs);
    bool operator>(const Contact& rhs);
    bool operator<(const Contact& rhs);
    bool operator!=(const Contact& rhs);
};
