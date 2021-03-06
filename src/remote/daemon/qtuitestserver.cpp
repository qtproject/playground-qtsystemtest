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


#include "qtuitestserver.h"
#include "qtuitest_debug.h"
#include "qtuitestdsettings.h"

#include <qtuitestslave.h>
#include <QCoreApplication>
#include <QStringList>
#include <QtSystemTest/QstDebug>

QtUiTestServer::QtUiTestServer(int port)
    : QTcpServer(), showDebug(false)
{
    QTT_TRACE_FUNCTION();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (!listen(QHostAddress::AnyIPv4, port)) {
#else
    if (!listen(QHostAddress::Any, port)) {
#endif
        qWarning() << "QtUiTestServer: couldn't listen for connections on " << port;
        exit(-1);
    }
    qLog() << "QtUiTestServer: listening for connections on" << port;
}

QtUiTestServer::~QtUiTestServer()
{
    QTT_TRACE_FUNCTION();
    close();
}

void QtUiTestServer::incomingConnection(int sd)
{
    QTT_TRACE_FUNCTION();
    qLog() << "New connection to qtuitestrunner on port" << sd;
    slave.setSocket(sd);
}

void QtUiTestServerManager::startService()
{
    QTT_TRACE_FUNCTION();
    qLog() << "QtUiTestServer: Starting service";

    int port = QtUiTestDSettings::instance()->port();
    int portMax = QtUiTestDSettings::instance()->portMax();

    for (int i=port; i<=portMax; ++i) {
        QtUiTestServer *server = new QtUiTestServer(i);
        m_servers << server;
    }
}

void QtUiTestServerManager::stopService()
{
    QTT_TRACE_FUNCTION();
    qLog() << "QtUiTestServer : Stoping service";

    foreach (QtUiTestServer *server, m_servers) {
        delete server;
    }
}
