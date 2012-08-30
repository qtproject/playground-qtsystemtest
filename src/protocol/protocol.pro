TEMPLATE = lib

TARGET = QstProtocol
TARGETPATH = QtSystemTest

TEMPLATE = lib

CONFIG += no_clean qt
QT += core network

mac:CONFIG += absolute_library_soname

SOURCES += \
    qstprotocol.cpp \
    qstconnection.cpp \
    qstconnectionmanager.cpp

PUBLIC_HEADERS += \
    qstprotocol.h \
    qstconnection.h \
    qstconnectionmanager.h \
    qstglobal.h \
    qstdebug.h \
    QstProtocol \
    QstConnection \
    QstConnectionManager \
    QstDebug

HEADERS = $$PUBLIC_HEADERS

# Installing libs and headers into Qt lib
includes.files = $$PUBLIC_HEADERS
includes.path = $$[QT_INSTALL_HEADERS]/$$TARGETPATH

target.path = $$[QT_INSTALL_LIBS]

INSTALLS += includes target

# Building so that other libs in project can access this lib
DESTDIR = ../../lib
INCLUDEDESTDIR = $$OUT_PWD/../../include/$$TARGETPATH
includedir.target = dummytarget
includedir.commands = (mkdir -p $$INCLUDEDESTDIR && cd $$PWD && cp $$PUBLIC_HEADERS $$INCLUDEDESTDIR/)

QMAKE_EXTRA_TARGETS += includedir

PRE_TARGETDEPS += dummytarget
