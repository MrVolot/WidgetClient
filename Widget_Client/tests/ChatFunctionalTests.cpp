#include "ChatFunctionalTests.h"
#include "../Chat.h"

ChatFunctionalTests::ChatFunctionalTests() {
}

void ChatFunctionalTests::test_hasSpaces() {
    Chat chat(0, "Test Friend", nullptr);
    //Test 1
    QVERIFY(chat.hasSpaces("Hello world"));
    //Test 2
    QVERIFY(!chat.hasSpaces("Helloworld"));
    //Test 3
    QVERIFY(chat.hasSpaces("  "));
    //Test 4
    QVERIFY(!chat.hasSpaces(""));
}

void ChatFunctionalTests::test_processMessage() {
    //Arrange
    Chat chat(0, "Test Friend", nullptr);
    //Test 1
    chat.processMessage("Hello world", true);
    QVERIFY(chat.vectorOfMessages.size() == 1);
    QVERIFY(chat.vectorOfMessages.at(0)->getMessageInfo().text == "Hello world");
    //Test 2
    chat.processMessage("Hello", false);
    QVERIFY(chat.vectorOfMessages.size() == 2);
    QVERIFY(chat.vectorOfMessages.at(1)->getMessageInfo().text == "Hello");
}

void ChatFunctionalTests::test_getClosestPunctuationMarkPosition() {
    Chat chat(0, "Test Friend", nullptr);
    //Test 1
    QCOMPARE(chat.getClosestPunctuationMarkPosition("Hello, world!", true), 0);
    //Test 2
    QCOMPARE(chat.getClosestPunctuationMarkPosition("Hello, world!", false), 12);
    //Test 3
    QCOMPARE(chat.getClosestPunctuationMarkPosition("Hello world", true), 0);
    //Test 4
    QCOMPARE(chat.getClosestPunctuationMarkPosition("Hello world", false), 0);
}

void ChatFunctionalTests::test_splitIntoMessages() {
    Chat chat(0, "Test Friend", nullptr);
    QString longMessage(2048, 'A');
    //Test 1
    chat.splitIntoMessages(longMessage, true);
    QVERIFY(chat.vectorOfMessages.size() == 2);
    QVERIFY(chat.vectorOfMessages.at(0)->getMessageInfo().text.size() == 1025);
    QVERIFY(chat.vectorOfMessages.at(1)->getMessageInfo().text.size() == 1023);
}

void ChatFunctionalTests::test_createWrap() {
    Chat chat(0, "Test Friend", nullptr);
    //Test 1
    QCOMPARE(chat.createWrap("Hello"), QString("Hello"));
    //Test 2
    QCOMPARE(chat.createWrap("This_is_a_very_long_word_without_spaces"), QString("This_is_a_very_long_word_witho\nut_spaces"));
    //Test 3
    QCOMPARE(chat.createWrap(""), QString(""));
}

QTEST_MAIN(ChatFunctionalTests)
