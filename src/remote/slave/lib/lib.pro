!include (../../remote.pri)

TEMPLATE = lib

TARGET = QstSlave
TARGETPATH = QtSystemTest

TEMPLATE = lib

CONFIG += no_clean qt
QT += core network widgets declarative quick

mac:CONFIG += absolute_library_soname

INCLUDEPATH += ../../../../include
LIBS += -L../../../../lib
LIBS += -lQstProtocol

contains(QT_MAJOR_VERSION, 5) {
    SOURCES += \
        qinputgenerator_qwindow.cpp \
        testquickobject.cpp \
        testquickitem.cpp \
        testquickview.cpp \
        testwindow.cpp

    PUBLIC_HEADERS += \
        testquickobject.h \
        testquickitem.h \
        testquickview.h \
        testwindow.h \

    HEADERS += \
        qinputgenerator_qwindow_p.h

    QT += qml qml-private quick gui-private core-private widgets
} else {
    INCLUDEPATH += $$QT_DECLARATIVE_PRIVATE_HEADERS_DIR
}

SOURCES += \
    qinputgenerator_generic.cpp \
    testquickfactory.cpp \
    testobject.cpp \
    testwidget.cpp \
    qasciikey.cpp \
    qtuitestconnectionmanager.cpp \
    qtuitestwidgetinterface.cpp \
    qeventwatcher.cpp \
    qtestslave.cpp \
    testapplication.cpp \
    testslaveserver.cpp \
    qtuitestwidgets.cpp \
    qinputgenerator.cpp \
    qtestwidgets.cpp \
    qtuitestnamespace.cpp

PUBLIC_HEADERS += \
    qtestwidgets.h \
    qtuitestnamespace.h \
    testquickfactory.h \
    testobject.h \
    testwidget.h \
    testapplication.h \
    qtuitestwidgetinterface.h \
    qtestslave.h \
    qtuitestmessagehandler.h \
    testslaveserver.h

HEADERS = \
    qinputgenerator_p.h \
    qtuitestconnectionmanager_p.h \
    qeventwatcher_p.h \
    qtuitestwidgets_p.h \
    qinputgenerator_generic_p.h \
    $$PUBLIC_HEADERS

# Installing libs and headers into Qt lib
includes.files = $$PUBLIC_HEADERS
includes.path = $$[QT_INSTALL_HEADERS]/$$TARGETPATH

target.path = $$[QT_INSTALL_LIBS]

INSTALLS += includes target

# Building so that other libs in project can access this lib
DESTDIR = ../../../../lib
INCLUDEDESTDIR = $$OUT_PWD/../../../../include/$$TARGETPATH
includedir.target = dummytarget
includedir.commands = (mkdir -p $$INCLUDEDESTDIR && cd $$PWD && cp $$PUBLIC_HEADERS $$INCLUDEDESTDIR/)

QMAKE_EXTRA_TARGETS += includedir

PRE_TARGETDEPS += dummytarget

