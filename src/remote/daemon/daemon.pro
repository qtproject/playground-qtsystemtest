!include (../remote.pri)

TEMPLATE = app
TARGET = qstdaemon
DESTDIR = ../../../bin

QT += network
QT -= gui

INCLUDEPATH += ../../../include
LIBS += -L../../../lib
LIBS += -lQstProtocol

HEADERS += \
    appmanager.h \
    qtuitest_debug.h \
    qtuitestdsettings.h \
    qtuitestmaster_p.h \
    qtuitestserver.h \
    qtuitestslave.h

SOURCES += \
    main.cpp \
    appmanager.cpp \
    qtuitestdsettings.cpp \
    qtuitestmaster.cpp \
    qtuitestserver.cpp \
    qtuitestslave.cpp

