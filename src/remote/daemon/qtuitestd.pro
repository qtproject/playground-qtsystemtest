!include(../../../global.pri): error(global.pri not found)
!include($$SRCROOT/src/common/common.pri): error(common.pri not found)

SOURCES += main.cpp \
           qtuitestslave.cpp \
           qtuitestserver.cpp \
           qtuitestmaster.cpp \
           appmanager.cpp \
           qtuitestdsettings.cpp

HEADERS += qtuitestslave.h \
           appmanager.h \
           qtuitestmaster_p.h \
           qtuitest_debug.h \
           qtuitestdsettings.h

TEMPLATE=app
VPATH+=$$PWD
INCLUDEPATH+=$$PWD

DEFINES += QTT_NO_DEBUG_AND_TRACEBACKS
!include(../../common/libs/QttDebug/qttdebug.pri): error(qttdebug.pri not found)

QT+=network
QT-=gui
TARGET=qtuitestd

unix {
    # install the script to start qtuitestd in the required
    # two places avoiding the need for symbolic links
    initscripts.path = /etc/init/test
    initscripts.files = initscripts/qtuitestd.conf
    # TODO: Disabled for now
    INSTALLS += initscripts
}

!symbian {
    MOC_DIR=$$OUT_PWD/.moc
    OBJECTS_DIR=$$OUT_PWD/.obj
    DESTDIR=$$BUILDROOT/bin
    target.path=$$QTT_INSTALL_BIN_DIR
    INSTALLS+=target
}

win32 {
    CONFIG+=console
}

mac {
    CONFIG-=app_bundle
}
