/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of QtSystemTest.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "appmanager.h"
#include "qtuitestslave.h"
#include "qtuitestmaster_p.h"
#include "qtuitest_debug.h"
#include "qtuitestdsettings.h"

#include <QDir>
#include <QVariant>
#include <QMetaProperty>
#include <QMetaObject>
#include <QMetaType>
#include <QProcess>
#include <QCoreApplication>
#include <QSettings>
#include <QPointer>

class QtUiTestSlavePrivate : public QObject
{
Q_OBJECT
public:
    QtUiTestSlavePrivate(QtUiTestSlave *parent)
        : p(parent)
    {
        AppManager::instance();
    }

    QString recursiveDelete(const QString&) const;

    QtUiTestSlave *p;
    AppManager *appManager()
    {
        return AppManager::instance();
    }

public slots:

    QstMessage currentApplication (const QstMessage&);
    QstMessage homeApplication    (const QstMessage&);
    QstMessage setApplication     (const QstMessage&);
    QstMessage isValidApplication (const QstMessage&);
    QstMessage getApplications    (const QstMessage&);

    QstMessage runProcess         (const QstMessage&);
    QstMessage getSetting         (const QstMessage&);
    QstMessage setSetting         (const QstMessage&);
    QstMessage putFile            (const QstMessage&);
    QstMessage getFile            (const QstMessage&);
    QstMessage deletePath         (const QstMessage&);
    QstMessage getDirectoryEntries(const QstMessage&);
    QstMessage checkOS            (const QstMessage&);
    QstMessage deviceConfig       (const QstMessage&);
    QstMessage systemTime         (const QstMessage&);
};

#include "qtuitestslave.moc"

QtUiTestSlave::QtUiTestSlave()
    : QstProtocol()
    , d(new QtUiTestSlavePrivate(this))
{
    setAutoUnpack(false);
    QObject::connect(static_cast<QstProtocol*>(this), SIGNAL(connectionClosed(QstProtocol*)), this, SLOT(onConnectionClosed()));
}

QtUiTestSlave::~QtUiTestSlave()
{
    delete d;
    disconnect();
}

void QtUiTestSlave::onConnected()
{
    QstProtocol::onConnected();
    QstMessage msg("AUT_CONNECT");
    msg["qtVersion"] = QT_VERSION_STR;
    msg["autVersion"] = QTUITEST_VERSION_STR;
    msg["autTime"] = QTime::currentTime();
    d->appManager()->testrunnerConnected(this);
    postMessage( msg );
}

void QtUiTestSlave::onConnectionClosed()
{
    qLog() << "Connection to qtuitestrunner has closed";
    d->appManager()->testrunnerDisconnected(this);
}

QstMessage QtUiTestSlavePrivate::currentApplication(const QstMessage &message)
{
    Q_UNUSED(message);
    QstMessage reply;

    reply[QLatin1String("currentApplication")] = appManager()->currentApplication();
    return reply;
}

QstMessage QtUiTestSlavePrivate::homeApplication(const QstMessage &message)
{
    Q_UNUSED(message);
    QstMessage reply;

    reply[QLatin1String("homeApplication")] = appManager()->homeApplication();
    return reply;
}

QstMessage QtUiTestSlavePrivate::setApplication(const QstMessage &message)
{
    QstMessage reply;
    QString appId = message.context();

    if (appId.isEmpty()) {
        return MESSAGE_ERROR(reply, QLatin1String("ERROR_MISSING_PARAMETERS"));
    }

    bool ok = appManager()->setApplication(appId);
    if (!ok) {
        return MESSAGE_ERROR(reply, QLatin1String("Unable to set application"));
    }
    return reply;
}

QstMessage QtUiTestSlavePrivate::isValidApplication(const QstMessage &message)
{
    QstMessage reply;
    QString appId = message.context();

    if (appId.isEmpty()) {
        return MESSAGE_ERROR(reply, QLatin1String("ERROR_MISSING_PARAMETERS"));
    }

    bool ok = appManager()->getApplications().contains(appId);
    reply[QLatin1String("isValidApplication")] = ok;
    return reply;
}

QstMessage QtUiTestSlavePrivate::getApplications(const QstMessage &message)
{
    Q_UNUSED(message);
    QstMessage reply;
    QString appName = message["name"].toString();

    reply[QLatin1String("getApplications")] = appManager()->getApplications(appName);
    return reply;
}

QstMessage QtUiTestSlavePrivate::runProcess(const QstMessage &message)
{
    QstMessage reply;

    QVariantMap cmd = message["cmd"].toMap();
    if (cmd.isEmpty()) {
        return MESSAGE_ERROR(reply, QLatin1String("ERROR_MISSING_PARAMETERS"));
    }

    QString program = cmd["program"].toString();
    QStringList arguments = cmd["arguments"].toStringList();
    int timeout = cmd["timeout"].toInt();
    QString directory = cmd["directory"].toString();
    QVariantMap environment = cmd["environment"].toMap();
    QVariantMap appendEnvironment = cmd["appendEnvironment"].toMap();
    bool detached = cmd["detached"].toBool();

    QProcessEnvironment env;
    if (!environment.isEmpty()) {
        foreach (const QString &key, environment.keys()) {
            env.insert(key, environment[key].toString());
        }
    }
    if (!appendEnvironment.isEmpty()) {
        if (env.isEmpty()) {
            env = QProcessEnvironment::systemEnvironment();
        }
        foreach (const QString &key, appendEnvironment.keys()) {
            env.insert(key, appendEnvironment[key].toString());
        }
    }

    bool ok = true;
    QString status;

    if (detached) {
        // Run command in background
        qint64 pid = 0;
        if (QProcess::startDetached(program, arguments, directory, &pid)) {
            status = QLatin1String("Started");
            reply["pid"] = pid;
        } else {
            status = QLatin1String("FailedToStart");
            ok = false;
        }
    } else {
        // Run command and wait for completion
        QString stdout;
        QString stderr;
        int exitCode = 0;
        QProcess *process = new QProcess(this);

        if (!env.isEmpty()) {
            process->setProcessEnvironment(env);
        }
        if (!directory.isEmpty()) {
            process->setWorkingDirectory(directory);
        }

        process->start(program, arguments);
        while (process->state() == QProcess::Starting) {
            QCoreApplication::processEvents();
        }

        if (process->error() == QProcess::FailedToStart) {
            status = QLatin1String("FailedToStart");
            ok = false;
        } else {
            QTime t;
            t.start();
            while (process->state() == QProcess::Running && t.elapsed() < timeout) {
                QCoreApplication::processEvents();
            }

            stdout += process->readAllStandardOutput();
            stderr += process->readAllStandardError();
            if (process->state() == QProcess::Running) {
                status = QLatin1String("TimedOut");
            } else if (process->exitStatus() != QProcess::NormalExit) {
                status = QLatin1String("Crashed");
                ok = false;
            } else {
                status = QLatin1String("Finished");
            }
        }

        exitCode = process->exitCode();
        delete process;

        reply["stdout"] = stdout;
        reply["stderr"] = stderr;
        reply["exitCode"] = exitCode;
    }
    qLog() << "Command status:" << status;
    reply["cmdStatus"] = status;

    if (!ok) {
        return MESSAGE_ERROR(reply, QString("Error: Command failed to run (%1)").arg(status));
    }
    return reply;
}

QstMessage QtUiTestSlavePrivate::getSetting(const QstMessage &message)
{
    QstMessage reply;
    QString org   = message[QLatin1String("org")].toString();
    QString app   = message[QLatin1String("app")].toString();
    QString path  = message[QLatin1String("path")].toString();
    QString group = message[QLatin1String("group")].toString();
    QString key   = message[QLatin1String("key")].toString();
    path = p->processEnvironment(path);
    if (group.isEmpty() || key.isEmpty()) {
        return MESSAGE_ERROR(reply, QLatin1String("ERROR_MISSING_PARAMETERS"));
    }
    if (!path.isEmpty() && (!org.isEmpty() || !app.isEmpty())) {
        return MESSAGE_ERROR(reply, QLatin1String("ERROR_BAD_PARAMETERS"));
    } else if (path.isEmpty() && org.isEmpty()) {
        return MESSAGE_ERROR(reply, QLatin1String("ERROR_MISSING_PARAMETERS"));
    }

    QSettings *settings = 0;
    if (!path.isEmpty()) settings = new QSettings(path, QSettings::NativeFormat);
    else                 settings = new QSettings(org, app);

    settings->beginGroup(group);
    reply[QLatin1String("getSetting")] = settings->value(key);
    delete settings;

    return reply;
}

QstMessage QtUiTestSlavePrivate::setSetting(const QstMessage &message)
{
    QstMessage reply;
    QString org   = message[QLatin1String("org")].toString();
    QString app   = message[QLatin1String("app")].toString();
    QString path  = message[QLatin1String("path")].toString();
    QString group = message[QLatin1String("group")].toString();
    QString key   = message[QLatin1String("key")].toString();
    path = p->processEnvironment(path);
    if (key.isEmpty()) {
        return MESSAGE_ERROR(reply, QLatin1String("ERROR_MISSING_PARAMETERS"));
    }
    if (!path.isEmpty() && (!org.isEmpty() || !app.isEmpty())) {
        return MESSAGE_ERROR(reply, QLatin1String("ERROR_BAD_PARAMETERS"));
    } else if (path.isEmpty() && org.isEmpty()) {
        return MESSAGE_ERROR(reply, QLatin1String("ERROR_MISSING_PARAMETERS"));
    }

    QSettings *settings = 0;
    if (!path.isEmpty()) settings = new QSettings(path, QSettings::NativeFormat);
    else                 settings = new QSettings(org, app);

    if (!group.isEmpty())
        settings->beginGroup(group);
    settings->setValue(key, message[QLatin1String("value")]);
    delete settings;

    return reply;
}

QstMessage QtUiTestSlavePrivate::putFile(const QstMessage &message)
{
    QstMessage reply;
    QString path = message[QLatin1String("path")].toString();
    path = p->processEnvironment(path);
    QByteArray data = message[QLatin1String("data")].toByteArray();

    if (path.isEmpty())
        return MESSAGE_ERROR(reply, QLatin1String("ERROR_MISSING_PARAMETERS"));

    {
        QFileInfo info = QFileInfo(path);
        QDir dir = info.dir();
        if (info.exists()) {
            dir.remove(info.fileName());
        } else if (!dir.exists() && !QDir(QLatin1String("/")).mkpath(dir.absolutePath())) {
            return MESSAGE_ERROR(reply, QLatin1String("Could not create path '") + dir.absolutePath() + QLatin1String("'"));
        }
    }

    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        if (message[QLatin1String("permissions")].isValid() && !f.setPermissions(static_cast<QFile::Permissions>(message[QLatin1String("permissions")].toInt()))) {
            return MESSAGE_ERROR(reply, QLatin1String("ERROR_FILE_PERMISSIONS"));
        }
        QDataStream ds(&f);
        if (data.constData()) {
            quint64 bytesWritten = ds.writeRawData(data.constData(), data.size());
            if ((qint64)bytesWritten == data.size())
                return reply;
            else {
                reply[QLatin1String("warning")] = QString("Wrote %1 byte(s), expected %2").arg(bytesWritten).arg(data.size());
                return MESSAGE_ERROR(reply, QLatin1String("ERROR_TRUNCATED"));
            }
        }
        // It's OK for data to be empty, then we create an empty file.
        return reply;
    }

    return MESSAGE_ERROR(reply, QLatin1String("ERROR_FILE_OPEN"));
}

QstMessage QtUiTestSlavePrivate::getFile(const QstMessage &message)
{
    QstMessage reply;
    QString path = message[QLatin1String("path")].toString();
    path = p->processEnvironment(path);

    if (path.isEmpty())
        return MESSAGE_ERROR(reply, QLatin1String("ERROR_MISSING_PARAMETERS"));

    QFile f(path);
    if (!f.exists()) return MESSAGE_ERROR(reply, QLatin1String("ERROR_FILE_NOT_EXIST"));
    if (f.open(QIODevice::ReadOnly)) {
        QByteArray data;
        QDataStream ds(&f);
        data.resize(f.size());
        quint64 bytesRead = ds.readRawData(data.data(), f.size());
        reply[QLatin1String("getFile")] = data;
        if ((qint64)bytesRead == f.size())
            return reply;
        else {
            reply[QLatin1String("warning")] = QString("Read %1 byte(s), expected %2").arg(bytesRead).arg(data.size());
            return MESSAGE_ERROR(reply, QLatin1String("ERROR_TRUNCATED"));
        }
    }
    return MESSAGE_ERROR(reply, QLatin1String("ERROR_FILE_OPEN"));
}

QstMessage QtUiTestSlavePrivate::getDirectoryEntries(const QstMessage &message)
{
    QstMessage reply;
    QString path = message[QLatin1String("path")].toString();
    path = p->processEnvironment(path);

    if (path.isEmpty()) {
        return MESSAGE_ERROR(reply, QLatin1String("ERROR_MISSING_PARAMETERS"));
    }

    QDir d(path);
    if (!d.exists()) {
        reply[QLatin1String("getDirectoryEntries")] = QStringList();
        return reply;
    }

    QDir::Filters filters;
    {
        int filters_int = message[QLatin1String("filters")].toInt();
        filters = QFlag(filters_int);
    }

    QStringList list = d.entryList(filters);
    reply[QLatin1String("getDirectoryEntries")] = list;
    return reply;
}

QstMessage QtUiTestSlavePrivate::deletePath(const QstMessage &message)
{
    QstMessage reply;
    QString path = message[QLatin1String("path")].toString();
    path = p->processEnvironment(path);
    QString deleteResult = recursiveDelete(path);
    if (deleteResult != QLatin1String("OK")) {
        return MESSAGE_ERROR(reply, deleteResult);
    }
    return reply;
}

void QtUiTestSlave::processMessage( QstMessage *msg )
{
    qLog() << "QtUiTestSlave::processMessage" << msg->event();

    QstMessage reply;
    QString methodSignature = msg->event() + QLatin1String("(QstMessage)");

    if (d->metaObject()->indexOfMethod(methodSignature.toLatin1().constData()) == -1) {
        if (!d->appManager()->isConnected()) {
            (void)MESSAGE_ERROR(reply, QLatin1String("ERROR: Not connected to test application"));
        } else if (!d->appManager()->forwardMessage(msg, this)) {
            (void)MESSAGE_ERROR(reply, QLatin1String("ERROR: Could not forward message '") + msg->event() + QLatin1String("'"));
        }

    } else {
        msg->unpack();
        if (!QMetaObject::invokeMethod(d, msg->event().toLatin1().constData(), Qt::DirectConnection,
                                    Q_RETURN_ARG(QstMessage, reply),
                                    Q_ARG(QstMessage, *msg))) {
            (void)MESSAGE_ERROR(reply, QLatin1String("ERROR: Failed to invoke method '") + msg->event() + QLatin1String("'"));
        }
        if (reply.state() != QstMessage::MessageError) {
            reply.setState(QstMessage::MessageSuccess);
        }
    }

    replyMessage(msg, reply);
}

QString QtUiTestSlave::processEnvironment( const QString& in ) const
{
    struct SystemEnvironment {
        static QMap<QString,QString> get() {
            QMap<QString,QString> ret;
            QStringList env = QProcess::systemEnvironment();
            foreach (const QString &str, env) {
                if (str.contains(QLatin1Char('='))) {
                    ret[str.left(str.indexOf(QLatin1Char('='))).toUpper()] = str.mid(str.indexOf(QLatin1Char('=')) + 1);
                }
            }
            return ret;
        }
    };
    static const QMap<QString,QString> environment( SystemEnvironment::get() );

    QString out;
    static QRegExp re("\\$[{(]?([A-Za-z0-9_]+)[})]?");
    int offset = 0;
    while (true) {
        int index = re.indexIn(in, offset);
        if (-1 == index) {
            out += in.mid(offset);
            break;
        }
        out += in.mid(offset, index - offset);
        out += environment.value(re.cap(1).toUpper());
        offset += re.matchedLength();
    }

    return out;
}

QString QtUiTestSlavePrivate::recursiveDelete( const QString &path ) const
{
    if (path.isEmpty()) return QLatin1String("ERROR_CANT_DELETE_EMPTY_PATH");

    QFileInfo i(path);
    if (!i.exists()) return QLatin1String("OK");
    if (!i.isDir() || i.isSymLink()) {
        if (!i.dir().remove(i.fileName())) {
            return QLatin1String("ERROR_CANT_DELETE_FILE_") + path;
        } else {
            return QLatin1String("OK");
        }
    }

    QDir dir(path);

    QStringList children = dir.entryList( QDir::AllEntries | QDir::System | QDir::Hidden | QDir::NoDotAndDotDot );

    QString res;
    foreach (QString child, children) {
        res = recursiveDelete( dir.absolutePath() + QLatin1String("/") + child );
        if (res != QLatin1String("OK") ) return res;
    }

    QString dirName = dir.dirName();
    dir.cdUp();

    if (!dir.rmdir(dirName)) {
        return QLatin1String("ERROR_CANT_DELETE_DIRECTORY_") + path;
    }
    return QLatin1String("OK");
}

QstMessage QtUiTestSlavePrivate::checkOS( const QstMessage &message )
{
    QstMessage reply;
    QString os = message[QLatin1String("os")].toString().toUpper();

    bool result = false;

#ifdef Q_OS_UNIX
    if (os == QLatin1String("UNIX")) result = true;
#endif
#ifdef Q_OS_LINUX
    if (os == QLatin1String("LINUX")) result = true;
#endif
#ifdef Q_OS_MAEMO
    if (os == QLatin1String("MAEMO")) result = true;
#endif
#ifdef Q_OS_MAC
    if (os == QLatin1String("MAC")) result = true;
#endif
#ifdef Q_OS_WIN32
    if (os == QLatin1String("WIN32")) result = true;
#endif
#ifdef Q_OS_WINCE
    if (os == QLatin1String("WINCE")) result = true;
#endif
#ifdef Q_OS_SYMBIAN
    if (os == QLatin1String("SYMBIAN")) result = true;
#endif

    reply[QLatin1String("checkOS")] = result;
    return reply;
}

QstMessage QtUiTestSlavePrivate::deviceConfig( const QstMessage &message )
{
    Q_UNUSED(message);
    QstMessage reply;
    reply[QLatin1String("deviceConfig")] = QtUiTestDSettings::instance()->deviceConfig();
    return reply;
}

QstMessage QtUiTestSlavePrivate::systemTime(const QstMessage &message)
{
    Q_UNUSED(message);
    QstMessage reply;

    reply["systemTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    return reply;
}
