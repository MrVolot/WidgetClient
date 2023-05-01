cmake_minimum_required(VERSION 3.5)

project(ChatTests LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Test)

set(TEST_SOURCES
    ChatFunctionalTests.cpp
    ChatFunctionalTests.h
    ../MessageWidget.cpp
    ../MessageWidget.h
    ../Mediator.h
    ../Mediator.cpp
    ../Chat.cpp
    ../Chat.h
    ../MessageContextMenu.h
    ../MessageContextMenu.cpp
    ../FileDropDialog.h
    ../FileDropDialog.cpp
    ../MessagesDateWidget.h
    ../MessagesDateWidget.cpp
    # Add other necessary source files here
)

add_executable(ChatTests ${TEST_SOURCES})
target_link_libraries(ChatTests PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Test)
