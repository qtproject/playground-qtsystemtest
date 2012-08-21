CONFIG += qt
QT += declarative declarative-private script

TARGET = qtsystemtestrunner
TEMPLATE = app

macx:CONFIG-=app_bundle

SOURCES += main.cpp\
    qstrunnercore.cpp \
    qstrunnerbootstrap.cpp \
    qstjsengine.cpp

HEADERS  += \
    qstrunnercore.h \
    qstrunnerbootstrap.h \
    qstjsengine.h

SCRIPTS += \
    scripts/QstRunner.js \
    scripts/QstRunner.qml

DESTDIR = ../../../bin
SCRIPTSDIR = $$OUT_PWD/../../../bin/scripts
scripts.target = dummytarget
scripts.commands = (mkdir -p $$SCRIPTSDIR && cd $$PWD && cp $$SCRIPTS $$SCRIPTSDIR/)

QMAKE_EXTRA_TARGETS += scripts

POST_TARGETDEPS += dummytarget
