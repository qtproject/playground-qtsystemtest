TARGET  = qtsystemtest
TARGETPATH = QtSystemTest


TEMPLATE = lib
CONFIG += no_clean qt plugin

QT += declarative declarative-private testlib

INCLUDEPATH += ../../../../include
LIBS += -L../../../../lib
LIBS += -lQstProtocol

SOURCES += src/imports.cpp \
    src/qstcoreapi.cpp \
    src/qstutils.cpp \
    src/qstlocalprocess.cpp

HEADERS += src/imports.h \
    src/qstcoreapi.h \
    src/qstutils.h \
    src/qstlocalprocess.h

OTHER_FILES += \
    qtsystemtest.json \

target.path += $$[QT_INSTALL_IMPORTS]/QtSystemTest

# public api
QML_IMPORT_FILES = \
    qml/TestMethod.qml \
    qml/TestItem.qml \
    qml/Test.qml \
    qml/TestCollection.qml \
    qml/Remote.qml \
    qml/Application.qml \
    qml/itemhandlers/QstObject.qml \
    qml/Js.qml \

qmlImportFiles.files += $$PWD/qml
qmlImportFiles.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH

# scripts
SCRIPT_FILES = \
    scripts/api.js \
    scripts/Datatype.js \
    scripts/Functional.js \
    scripts/JsCore.js \
    scripts/SourceBuilder.js \
    scripts/Class.js \
    scripts/QstCore.js \
    scripts/Console.js \
    scripts/Expect.js \
    scripts/Exception.js \
    scripts/ComponentCacher.js \
    scripts/ApplicationRegistry.js \
    scripts/ItemFactory.js \

scriptFiles.files += $$SCRIPT_FILES
scriptFiles.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH/scripts

# qmldir
QMLDIR_FILE = qmldir
qmldirFile.files += $$QMLDIR_FILE
qmldirFile.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH

# show in QtCreator
OTHER_FILES += $$QML_IMPORT_FILES \
    $$SCRIPT_FILES \
    qmldir \


INSTALLS +=  qmlImportFiles scriptFiles qmldirFile target

