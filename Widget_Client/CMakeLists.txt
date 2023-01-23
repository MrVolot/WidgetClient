cmake_minimum_required(VERSION 3.5)

project(Widget_Client VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        Mainwindow.cpp
        Mainwindow.h
        Mainwindow.ui
        RegisterDialog.cpp
        RegisterDialog.h
        RegisterDialog.ui
        Commands.h
        Config.h
        Config.cpp
        ../Json/jsoncpp.cpp
        ../Json/json/json.h
        Contact.h
        Contact.cpp
        IoServiceWorker.h
        IoServiceWorker.cpp
        OnlineStatusChecker.h
        OnlineStatusChecker.cpp
        WindowManager.h
        WindowManager.cpp
        Messenger.h
        Chat.h
        Chat.cpp
        Chat.ui
        ContactsWidget.h
        ContactsWidget.cpp
        ContactsWidget.ui
        MessageWidget.h
        MessageWidget.cpp
        MessageWidget.ui
        MessagesDateWidget.h
        MessagesDateWidget.cpp
        MessagesDateWidget.ui
)

set(BOOST_LIBRARYDIR D:/Boost/boost-build/lib)
set(BOOST_ROOT D:/Boost/boost_1_78_0)

set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED ON)

FIND_PACKAGE(Boost)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
    message("boost found")
else()
    message("Boost not found")
ENDIF()

set (BOOST_COMPONENTS system thread)

find_package (Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Widget_Client
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Widget_Client APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Widget_Client SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Widget_Client
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Widget_Client PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(Widget_Client PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS Widget_Client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Widget_Client)
endif()
