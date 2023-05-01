#ifndef CHATFUNCTIONALTESTS_H
#define CHATFUNCTIONALTESTS_H

#include <QObject>
#include <QtTest/QtTest>

class ChatFunctionalTests : public QObject {
    Q_OBJECT

public:
    ChatFunctionalTests();

private slots:
    void test_hasSpaces();
    void test_processMessage();
    void test_getClosestPunctuationMarkPosition();
    void test_splitIntoMessages();
    void test_createWrap();
};

#endif // CHATFUNCTIONALTESTS_H
