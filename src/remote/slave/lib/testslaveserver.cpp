/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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


#include "testslaveserver.h"
// #include "qsensorgenerator_p.h"
#include <qtestslave.h>
#include <QCoreApplication>
#include <QStringList>
#include <QLocalSocket>
#include <QtSystemTest/QstDebug>

#ifdef Q_OS_SYMBIAN
#  include <sys/socket.h>
#  include <net/if.h>
#endif

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
extern Q_GUI_EXPORT bool qt_use_native_dialogs;
#endif

TestSlaveServer::TestSlaveServer()
    : QTcpServer(), showDebug(false), started(false), m_useDaemon(false)
{
    QTT_TRACE_FUNCTION();
}

TestSlaveServer::~TestSlaveServer()
{
    QTT_TRACE_FUNCTION();
    close();
}

void TestSlaveServer::incomingConnection(int sd)
{
    QTT_TRACE_FUNCTION();
    slave.setSocket(sd);
}

quint16 TestSlaveServer::queryDaemonForPort() const
{
    QTT_TRACE_FUNCTION();
    QLocalSocket socket;
    socket.connectToServer(QString::fromLocal8Bit(QTUITEST_PORT_PATH));
    qDebug() << "qtuitest: Getting port...";
    if (!socket.waitForConnected(500)) {
        return 0;
    }

    char buf[32];
    qDebug() << "qtuitest: Got local socket..." << &socket;
    if (!socket.waitForReadyRead(5000)) {
        socket.close();
        return 0;
    }

    qint64 length = socket.readLine(buf, sizeof(buf));
    socket.close();

    if (length == -1) {
        return 0;
    }

    bool ok = false;
    quint16 port = QString(buf).toUShort(&ok);
    if (!ok) {
        return 0;
    }

    qDebug() << "qtuitest: Received Port" << port;
    return port;
}

void TestSlaveServer::initSlave(bool useDaemon)
{
    QTT_TRACE_FUNCTION();
    m_useDaemon = useDaemon;

//     QSensorGeneratorFactory *factory = new QSensorGeneratorFactory;
//     factory->registerSensors();

    QTimer::singleShot(100, this, SLOT(startService()));
}

void TestSlaveServer::startService()
{
    QTT_TRACE_FUNCTION();
    bool parseOk;
    QStringList args = QCoreApplication::arguments();
    QByteArray debugEnvBa = qgetenv("QTUITEST_DEBUG");
    QString debugEnv = QString(debugEnvBa);

#ifndef QTUITEST_DEBUG
    int debugValue = debugEnv.toInt(&parseOk);
    if (parseOk && (debugValue > 0))
        showDebug = true;
#else
    showDebug = true;
#endif

    if (showDebug)
        qWarning("QtUiTest: Start service");

    for (int index = 0; index < args.count(); index++){
        if (args[index].endsWith("qtuitestrunner")) {
            if (showDebug)
                qWarning() << "QtUiTest: Not starting TestSlaveServer::startService for qtuitestrunner";
            return;
        }
    }

    if (started) {
        if (showDebug)
            qWarning() << "QtUiTest: Closing TestSlaveServer connection";

        close();
    }

    quint16 aut_port = DEFAULT_AUT_PORT;

    if (m_useDaemon) {
        aut_port = queryDaemonForPort();
        if (!aut_port) {
            qDebug() << "Not received port, will retry in 2000ms";
            QTimer::singleShot(2000, this, SLOT(startService()));
            return;
        }
    } else {
        QString autPortEnv = QString::fromLocal8Bit(qgetenv("QTUITEST_PORT"));

        if (!autPortEnv.isEmpty()) {
            aut_port = autPortEnv.toUShort(&parseOk);
            if (!parseOk) {
                aut_port = DEFAULT_AUT_PORT;
                qWarning() << "QtUiTest: Unable to parse QTUITEST_PORT" << autPortEnv;
            } else {
                if (showDebug)
                    qWarning() << "QtUiTest: Set port via QTUITEST_PORT to" << aut_port;
            }
        }
        for (int index = 0; index < args.count(); index++) {
            if ((args[index] == QLatin1String("-autport")) && (index + 1 < args.count())) {
                aut_port = args[index + 1].toUShort(&parseOk);
                if (!parseOk) {
                    aut_port = DEFAULT_AUT_PORT;
                    qWarning() << "QtUiTest: Unable to parse -autport" << args[index];
                } else {
                    if (showDebug)
                        qWarning() << "QtUiTest: Set port via -autport to" << aut_port;
                }
            }
        }
    }

    m_port = aut_port;

    QTimer::singleShot(0, this, SLOT(initConnection()));
}

void TestSlaveServer::initConnection()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (!listen(QHostAddress::AnyIPv4, m_port)) {
#else
    if (!listen(QHostAddress::Any, m_port)) {
#endif
        qWarning() << "QtUiTest: couldn't listen for connections on " << m_port << ":" << errorString();
        return;
    }

    started = true;
    if (showDebug)
        qWarning() << "QtUiTest: listening for connections on" << m_port;
}

void TestSlaveServer::stopService()
{
    QTT_TRACE_FUNCTION();
    if (showDebug)
        qWarning("QtUiTest : Stoping service");

    close();
    started = false;
}
