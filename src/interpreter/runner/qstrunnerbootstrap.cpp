/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of QtSystemTest.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qstrunnerbootstrap.h"
#include "qstrunnercore.h"
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptContextInfo>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>

static const int InternalError = 1 << 6;


static QScriptValue isRelativePath(QScriptContext *ctx, QScriptEngine *)
{
    QUrl url = ctx->argument(0).toString();
    return url.isRelative();
}

static QScriptValue relativeToCurrentPath(QScriptContext *ctx, QScriptEngine *)
{
    QUrl url = ctx->argument(0).toString();
    if (url.isLocalFile())
        return QDir::current().relativeFilePath(url.toLocalFile());
    return url.toString();
}


static QScriptValue qscriptengine_stack(QScriptContext *ctx, QScriptEngine *e)
{
    QVariantList stack;

    qint32 frame = 0;
    bool framesSpecified = ctx->argumentCount() > 0;
    if (framesSpecified) {
        frame = ctx->argument(0).toInt32();
    }

    while (ctx) {
        // count down if user has requested for certain number of frames
        if ( framesSpecified &&  frame-- <= 0 )
            break;
        ctx = ctx->parentContext();
        QScriptContextInfo info(ctx);
        QVariantMap callsite;
        callsite["fileName"] = info.fileName();
        callsite["lineNumber"] = info.lineNumber();
        callsite["functionName"] = info.functionName();
        stack << callsite;
    }
    return qScriptValueFromSequence(e, stack);
}

// Copied from qdeclarativedebughelper_p.h in Qt
class Q_DECLARATIVE_EXPORT QDeclarativeDebugHelper
{
public:
    static QScriptEngine *getScriptEngine(QDeclarativeEngine *engine);
};

bool QstRunnerBootstrap::setupScriptEngine(QDeclarativeEngine *declarativeEngine)
{
    QScriptEngine *e = QDeclarativeDebugHelper::getScriptEngine(declarativeEngine);
    QScriptValue qst = e->newObject();
    qst.setProperty(QLatin1String("stack"), e->newFunction(qscriptengine_stack));
    qst.setProperty(QLatin1String("isRelativePath"), e->newFunction(isRelativePath));
    qst.setProperty(QLatin1String("relativeToCurrentPath"), e->newFunction(relativeToCurrentPath));

    QScriptValue qtObject =  e->globalObject().property(QLatin1String("Qt"));
    qtObject.setProperty(QLatin1String("qst"), qst);
    return true;
}

QstRunnerBootstrap::QstRunnerBootstrap(QCoreApplication *app)
        : QObject(app)
        , m_app(app)
        , m_qmlRunner(0)
{
    QStringList args = app->arguments();
    // Skips executable
    args.removeAt(0);

    if (args.count() == 0) {
        qDebug() << "No arguments passed, failing back to qmlunit Test Suite (" << app->applicationDirPath() << ")\n";
        args = QStringList(app->applicationDirPath() + "/test");
    }

    foreach (const QString &arg, args) {
        findTests(arg);
    }
}


bool QstRunnerBootstrap::setup()
{
    qmlRegisterType<QstRunnerCore>("QstRunner", 1, 0, "Runner");
    QDeclarativeEngine *engine = new QDeclarativeEngine(this);
    engine->setOfflineStoragePath(QDir::currentPath() + "/storage");
    setupScriptEngine(engine);

    qDebug()<< "Loading Runner";
    QUrl runnerPath = QUrl::fromLocalFile(
                m_app->applicationDirPath() + "/scripts/QstRunner.qml");

    QDeclarativeComponent runnerCom(engine, runnerPath);
    if (!runnerCom.errors().isEmpty()) {
        qDebug() << "Internal error in Qml runner"
                    << runnerCom.errors();
        return false; // may be exit 2
    }
    m_qmlRunner = qobject_cast<QstRunnerCore *>(runnerCom.create());
    if (!m_qmlRunner) {
        qDebug() << "Internal Error: qmlRunner creation failed"
                    << runnerCom.errors();
        return false; // may be exit 2
    }

    if ( !m_qmlRunner->setup(engine, qApp->arguments()) ) {
        return false;
    }

    qDebug()<< "Loading test file";
    QDeclarativeComponent com(engine, QUrl::fromLocalFile(m_testFile.first()));
    if (!com.errors().isEmpty()) {
        qDebug() << "Errors in testcase \n"
                    << com.errors();
        return false; // may be exit 1
    }
    QObject *test = com.create();
    if (!test ) {
        qDebug() << "Internal Error testcase object creation \n"
                    << com.errors();
        return false; // may be exit 1
    }
    m_qmlRunner->setTest(test);
    return true;
}

int QstRunnerBootstrap::run()
{
    if (!setup()) {
        return InternalError;
    }
    qDebug() << Q_FUNC_INFO << "---------------[setup is ok]-------------------";
    return m_qmlRunner->exec();
}

void QstRunnerBootstrap::findTests(const QString &path)
{
    if (isTest(path)) {
        m_testFile << QDir(path).absolutePath();
        return;
    }

    QStringList filters; filters << "*";
    QDir dir = QDir(QDir(path).absolutePath());

    QListIterator<QFileInfo> files(dir.entryInfoList(filters, QDir::AllEntries | QDir::NoDotAndDotDot));
    while(files.hasNext()) {
        QFileInfo file = files.next();
        if (file.fileName() == "." || file.fileName() == "..") continue;

        if (isTest(file))
            m_testFile << file.absoluteFilePath();
        else if (isDir(file))
            findTests(file.absoluteFilePath());
    }
}

bool QstRunnerBootstrap::isTest(const QFileInfo &file)
{
    return isTest(file.fileName());
}

bool QstRunnerBootstrap::isTest(const QString &filePath)
{
    return filePath.endsWith("Test.qml")
            || filePath.endsWith("Tests.qml");
}

bool QstRunnerBootstrap::isDir(const QFileInfo &file)
{
    return QDir(file.absoluteFilePath()).exists();
}
