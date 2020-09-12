QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

QT += printsupport

QT += charts

TARGET = amp
TEMPLATE = app

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    OrderData.cpp \
    TableviewEx/buttondelegate.cpp \
    TableviewEx/tablemodel.cpp \
    aboutdialog.cpp \
    appordersframe.cpp \
    cashboxsetframe.cpp \
    consumerdisplayframe.cpp \
    consumertypeframe.cpp \
    cookingtimeframe.cpp \
    datathread.cpp \
    estimateclearlyframe.cpp \
    flavorsettingframe.cpp \
    gratuityframe.cpp \
    gridlayoutex.cpp \
    homeframe.cpp \
    installwizard.cpp \
    launchframe.cpp \
    loginframe.cpp \
    main.cpp \
    mainwindow.cpp \
    menuframe.cpp \
    menuitemexframe.cpp \
    moresettingframe.cpp \
    orderpainter.cpp \
    orderpayframe.cpp \
    ordersframe.cpp \
    ordersmanagerframe.cpp \
    posbuttonex.cpp \
    printersframe.cpp \
    qrcode/BitBuffer.cpp \
    qrcode/QrCode.cpp \
    qrcode/QrSegment.cpp \
    qrcodepayframe.cpp \
    recommendationframe.cpp \
    redis-cli/command.cpp \
    redis-cli/redisclient.cpp \
    redis-cli/reply.cpp \
    reducemenupriceframe.cpp \
    reportsframe.cpp \
    settinghomeframe.cpp \
    shopcartview.cpp \
    sysoptframe.cpp \
    tablesframe.cpp \
    tabviewpagerframe.cpp \
    version.cpp \
    vkeyboardex.cpp

HEADERS += \
    OrderData.h \
    TableviewEx/buttondelegate.h \
    TableviewEx/tablemodel.h \
    aboutdialog.h \
    appordersframe.h \
    cashboxsetframe.h \
    consumerdisplayframe.h \
    consumertypeframe.h \
    cookingtimeframe.h \
    datathread.h \
    estimateclearlyframe.h \
    flavorsettingframe.h \
    gratuityframe.h \
    gridlayoutex.h \
    homeframe.h \
    installwizard.h \
    launchframe.h \
    loginframe.h \
    mainwindow.h \
    menuframe.h \
    menuitemexframe.h \
    moresettingframe.h \
    orderpainter.h \
    orderpayframe.h \
    ordersframe.h \
    ordersmanagerframe.h \
    posbuttonex.h \
    printersframe.h \
    qrcode/BitBuffer.hpp \
    qrcode/QrCode.hpp \
    qrcode/QrSegment.hpp \
    qrcodepayframe.h \
    recommendationframe.h \
    redis-cli/command.h \
    redis-cli/redisclient.h \
    redis-cli/reply.h \
    reducemenupriceframe.h \
    reportsframe.h \
    settinghomeframe.h \
    shopcartview.h \
    shopcartview.h \
    sysoptframe.h \
    tablesframe.h \
    tabviewpagerframe.h \
    version.h \
    vkeyboardex.h

FORMS += \
    aboutdialog.ui \
    mainwindow.ui \
    qrcodepayframe.ui

TRANSLATIONS += pos-app-zh.ts \
                pos-app-en.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

INCLUDEPATH += $$PWD/libs
DEPENDPATH += $$PWD/libs


unix: LIBS += -L$$PWD/../../../../usr/lib/ -lcurl
android: LIBS += -L$$PWD/libcurl-armeabi-v7a/lib -lcurl

unix: INCLUDEPATH += $$PWD/../../../../usr/lib

android: INCLUDEPATH += $$PWD/libcurl-armeabi-v7a/include


DEPENDPATH += $$PWD/../../../../usr/lib

DISTFILES += \
    qrcode/qrencode.1.in


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libcurl-for-win32/lib/ -lcurl
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libcurl-for-win32/lib/ -lcurl

INCLUDEPATH += $$PWD/libcurl-for-win32/include
DEPENDPATH += $$PWD/libcurl-for-win32/include
