TEMPLATE=lib
CONFIG += plugin

TARGET=qststyle
TARGET=$$qtLibraryTarget($$TARGET)

QT+=network

INCLUDEPATH += ../../../../include
LIBS += -L../../../../lib
LIBS += -lQstProtocol -lQstSlave

DESTDIR=../../../../plugins

SOURCES+=qtuiteststyleplugin.cpp

target.path=$$[QT_INSTALL_PLUGINS]/styles
INSTALLS+=target
