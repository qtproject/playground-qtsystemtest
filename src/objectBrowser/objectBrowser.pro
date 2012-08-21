TEMPLATE = app
TARGET=qstobjectbrowser
DESTDIR=../../bin

QT+=network

contains(QT_MAJOR_VERSION, 5) {
  QT+=widgets
}

INCLUDEPATH += ../../include
LIBS += -L../../lib
LIBS += -lQstProtocol

HEADERS +=\
        qtestbrowserdlg_p.h

SOURCES +=\
        qtestbrowserdlg.cpp \
        main.cpp
