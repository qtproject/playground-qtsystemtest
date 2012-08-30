TARGET = testapp_widgets
TEMPLATE = app
DESTDIR = ../../bin

macx:CONFIG-=app_bundle

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
