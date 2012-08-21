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
#include "qtuitestmaster_p.h"
#include "qtuitestdsettings.h"
#include "qtuitest_debug.h"

#include <qtuitestslave.h>
#include <QLocalServer>
#include <QLocalSocket>

TestApplication::TestApplication(const QString &appId, AppManager *appManager)
    : m_appId(appId)
    , m_port(0)
    , m_env()
    , m_connection(0)
    , m_appManager(appManager)
    , m_state(NotStarted)
{
}

TestApplication::~TestApplication()
{
    m_connection->deleteLater();
}

void TestApplication::onConnectionClosed()
{
    if (m_state == Disconnected)
        return;

    qLog() << "Connection to application closed" << this << m_state;
    if (m_state == ProcessingMessage) {
        qLog() << "Still processing message";
        m_state = Disconnected;
        return;
    }
    m_state = Disconnected;
    emit finished();
}

void TestApplication::onSignalReceived(const QVariantMap &sender, const QString &signal, const QVariantList &args)
{
    QstMessage msg("SIGNAL_RECEIVED");
    msg.setContext(m_appId);
    msg["sender"] = sender;
    msg["signal"] = signal;
    msg["args"] = args;
    m_appManager->postMessage( msg );
}

bool TestApplication::forwardMessage( QstMessage *msg, QtUiTestSlave *slave )
{
    qLog() << "TestApplication::forwardMessage" << msg->event() << "to" << m_appId << "on port" << m_port;

    if (m_connection) {
        QstMessage reply;
        m_state = ProcessingMessage;
        m_connection->sendMessage(*msg, reply, 20000);
        reply.setContext(m_appId);
        slave->replyMessage(msg, reply);
        if (m_state == Disconnected) {
            qLog() << "Connection closed while processing message";
            emit finished();
            return false;
        }
        m_state = Connected;
        return true;
    }
    return false;
}

bool TestApplication::connectToAut(int timeout)
{
    if (!m_connection) {
        m_connection = new QtUiTestMaster();
        connect(m_connection, SIGNAL(signalReceived(QVariantMap,QString,QVariantList)), this, SLOT(onSignalReceived(QVariantMap,QString,QVariantList)));
    }

    QTime t;
    t.start();
    while (t.elapsed() < timeout && !m_connection->isConnected()) {
        m_connection->connect( QString("127.0.0.1"), m_port );
        m_connection->waitForConnected(2000);
    }

    if (!m_connection->isConnected()) {
        qLog() << "Failed to connect to" << m_appId << "on port" << m_port;
        qLog() << "Connection state" << m_connection->state();
        if (m_connection->state() == QAbstractSocket::ConnectedState) {
            // Connected, but no response!
            qLog() << "Connected but did not receive reply";
            m_connection->disconnect();
            emit finished(false);
        } else {
            emit finished();
        }
        return false;
    }
    qLog() << "Connected to" << m_appId << "on port" << m_port;
    m_state = Connected;

    // Don't try to reconnect if connection is lost ... it's pointless
    m_connection->enableReconnect(false, 0);
    connect(static_cast<QstProtocol*>(m_connection), SIGNAL(connectionClosed(QstProtocol*)), this, SLOT(onConnectionClosed()));

    QstMessage msg("appName");
    QstMessage response;
    m_connection->sendMessage( msg, response, 2000 );
    response.unpack();
    m_appName = response["appName"].toString();
    qLog() << "Application name is" << m_appName;
    if (!m_appName.isEmpty() && m_appName == QtUiTestDSettings::instance()->homeApp()) {
        qLog() << "This is the home application";
        m_appManager->setDefaultApplication(this);
    }

    return true;
}

bool TestApplication::isConnected() const
{
    if (m_connection) {
        return m_connection->isConnected();
    }
    return false;
}

void TestApplication::cleanup()
{
    QstMessage msg("cleanup");
    m_connection->postMessage(msg);
}

AppManager::AppManager()
    : m_currentApplication(0)
    , m_defaultApplication(0)
    , m_server(0)
{
    QLocalServer::removeServer(QString::fromLocal8Bit(QTUITEST_PORT_PATH));
    m_server = new QLocalServer(this);
    m_server->listen(QString::fromLocal8Bit(QTUITEST_PORT_PATH));
    connect(m_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

    if (int port = QtUiTestDSettings::instance()->prestartPort()) {
        qLog() << "Trying to connect to existing application on port" << port;
        m_defaultApplication = new TestApplication(QtUiTestDSettings::instance()->prestartApp(), this);
        if (!m_defaultApplication || !connectApplication(port, m_defaultApplication)) {
            qWarning() << "Failed to connect to existing application on port" << port;
            exit(1);
        }
    }
}

AppManager::~AppManager()
{
}

AppManager* AppManager::instance() {
    static AppManager appManager;
    return &appManager;
}

bool AppManager::connectApplication(int port, TestApplication *app, int timeout)
{
    connect(app, SIGNAL(finished(bool)), this, SLOT(onFinished(bool)));

    app->setPort(port);
    if (!app->connectToAut(timeout))
        return false;

    m_portMap.insert(port, app);
    m_appMap.insert(app->appId(), app);
    m_pendingPorts.removeAll(port);

    if (app == m_defaultApplication || QtUiTestDSettings::instance()->homeApp().isEmpty()) {
        setApplication(app);
    }

    return true;
}

void AppManager::purgeApplications()
{
    foreach (const QString &appId, m_appMap.keys()) {
        m_appMap[appId]->deleteLater();
    }

    m_appMap.clear();
}

bool AppManager::forwardMessage( QstMessage *msg, QtUiTestSlave *slave )
{
    QString appId = msg->context();
    TestApplication *app;

    if (appId.isEmpty()) {
        app = m_currentApplication;
    } else {
        app = m_appMap[appId];
        if (app) {
            qLog() << "Forwarding message to application id" << appId;
        } else {
            qLog() << "Cannot forward to application id" << appId;
        }
    }

    if (app) {
        return app->forwardMessage(msg, slave);
    }
    return false;
}

void AppManager::onNewConnection()
{
    qLog() << "New application requesting connection";
    int port = getFreePort();
    m_pendingPorts.append(port);
    QString portString = QString::number(port).toLatin1() + QLatin1Char('\n');
    qLog() << "Assigning port:" << port;
    QLocalSocket *socket = m_server->nextPendingConnection();
    socket->write(portString.toLatin1());
    socket->disconnect();
    socket->deleteLater();

    //FIXME what identifier to use?
    static int counter=0;
    QString application = QString("app_%1:%2").arg(counter++, 5, 10, QLatin1Char('0')).arg(port);
    TestApplication *app = new TestApplication(application, this);
    if (!connectApplication(port, app)) {
        qWarning() << "Failed to connect to application on port" << port;
        return;
    }

    // Send message to notify test runner of new application
    QstMessage msg("APP_STARTED");
    msg.setContext(application);
    postMessage( msg );
}

void AppManager::onFinished(bool reusePort)
{
    TestApplication *app = qobject_cast<TestApplication*>(sender());

    if (app) {
        qLog() << "Finished" << app->appId();

        // Send message to notify test runner that app has closed
        QstMessage msg("APP_FINISHED");
        msg.setContext(app->appId());
        if (app == m_defaultApplication) {
            msg["homeapp"] = QtUiTestDSettings::instance()->homeApp();
        }
        postMessage( msg );

        if (m_defaultApplication && app == m_currentApplication)
            setApplication(m_defaultApplication);

        if (reusePort) {
            m_portMap.remove(app->port());
        }

        m_appMap.remove(app->appId());
        qLog() << "Apps now: " << m_appMap.keys();

        app->deleteLater();

        if (app == m_defaultApplication) {
            // The home application has died!
            qLog() << "The home application has terminated!";
            m_defaultApplication = 0;
            purgeApplications();
            return;
        }

    }
}

int AppManager::getFreePort() const
{
    int port = QtUiTestDSettings::instance()->appPortMin();

    //Find first unused port in range
    foreach (int usedPort, m_portMap.keys()) {
        if ((port != usedPort)
            && !(m_pendingPorts.contains(port))) {
            break;
        }
        port++;
    }
    if (m_pendingPorts.contains(port)) {
        port++;
    }

    if (port > QtUiTestDSettings::instance()->appPortMax()) {
        qWarning("Exceeded maximum port number");
    }

    return port;
}

bool AppManager::isConnected() const
{
    if (m_currentApplication) {
        return m_currentApplication->isConnected();
    }
    return false;
}

void AppManager::setDefaultApplication(TestApplication *app)
{
    m_defaultApplication = app;
}

QString AppManager::currentApplication() const
{
    QString ret;
    if (m_currentApplication) {
        ret = m_currentApplication->appId();
    }
    return ret;
}

QString AppManager::homeApplication() const
{
    QString ret;
    if (m_defaultApplication) {
        ret = m_defaultApplication->appId();
    }
    return ret;
}

QStringList AppManager::getApplications(const QString &name)
{
    QStringList result;
    foreach (const QString &appId, m_appMap.keys()) {
        if (!m_appMap[appId])
            m_appMap.remove(appId);
        if (name.isEmpty() || m_appMap[appId]->appName() == name)
            result.append(appId);
    }
    return result;
}

bool AppManager::setApplication(const QString &appId)
{
    TestApplication *app = m_appMap[appId];
    if (!app) {
        return false;
    }

    setApplication(app);
    return true;
}

void AppManager::setApplication(TestApplication *app)
{
    qLog() << "Setting current application to" << app << app->appId();
    m_currentApplication = app;
}

void AppManager::postMessage(const QstMessage &msg)
{
    //FIXME - should inform all runners
    foreach (QtUiTestSlave *slave, m_slaves) {
        slave->postMessage( msg );
    }
}

void AppManager::testrunnerConnected(QtUiTestSlave *slave)
{
    if (!m_slaves.contains(slave)) {
        m_slaves << slave;
    }
}

void AppManager::testrunnerDisconnected(QtUiTestSlave *slave)
{
    if (m_slaves.contains(slave)) {
        m_slaves.removeAll(slave);
    }

    if (m_slaves.isEmpty()) {
        foreach (QPointer<TestApplication> app, m_appMap.values()) {
            if (app)
                app->cleanup();
        }
    }
}
