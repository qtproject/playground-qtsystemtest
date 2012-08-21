TARGET  = qtsystemtestrunner
TARGETPATH = QtSystemTest/Runner

TEMPLATE = lib
CONFIG += no_clean qt plugin
QT += declarative declarative-private


SOURCES += src/imports.cpp \
    src/qstxunitwriter.cpp \
    src/qstlogwriter.cpp \

HEADERS += src/imports.h \
    src/qstxunitwriter.h \
    src/qstlogwriter.h \

OTHER_FILES += \
    qtsystemtest.json \

target.path += $$[QT_INSTALL_IMPORTS]/QtSystemTest/Runner

# public api
QML_IMPORT_FILES = \
    qml/Config.qml \
    qml/Logger.qml \
    qml/PlainTextLogger.qml \
    qml/XmlLogger.qml \
    qml/XUnitLogger.qml

qmlImportFiles.files += $$QML_IMPORT_FILES
qmlImportFiles.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH/qml

# scripts
#SCRIPT_FILES = \
#    scripts/api.js \
#    scripts/Datatype.js \
#    scripts/Functional.js \
#    scripts/JsCore.js \
#    scripts/SourceBuilder.js \
#    scripts/Class.js \
#    scripts/QstCore.js \
#    scripts/Console.js \
#    scripts/Expect.js \
#    scripts/Exception.js \
#    scripts/ApplicationRegistry.js \

#scriptFiles.files += $$SCRIPT_FILES
#scriptFiles.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH/scripts

# qmldir
QMLDIR_FILE = qmldir
qmldirFile.files += $$QMLDIR_FILE
qmldirFile.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH

# show in QtCreator
OTHER_FILES += $$QML_IMPORT_FILES \
#    $$SCRIPT_FILES \
    qmldir \


INSTALLS +=  qmlImportFiles qmldirFile target









