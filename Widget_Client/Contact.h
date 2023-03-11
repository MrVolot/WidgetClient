#pragma once
#include <qstring.h>
#include <string>

class Contact
{
    QString name_;
    unsigned long long id_;
    std::pair<unsigned long long, QString> lastMessageInfo_;
public:
    Contact(const QString& name, unsigned long long id, std::pair<unsigned long long, QString> lastMessageInfo);
    QString& getName();
    unsigned long long getId();
    std::pair<unsigned long long, const QString&> getLastMessageInfo();

    bool operator==(const Contact& rhs);
    bool operator>(const Contact& rhs);
    bool operator<(const Contact& rhs);
    bool operator!=(const Contact& rhs);
};
