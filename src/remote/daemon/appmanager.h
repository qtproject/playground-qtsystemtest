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


#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <QStringList>
#include <QPointer>
#include <QVariantMap>
#include <QMap>

class AppManager;
class QtUiTestMaster;
class QLocalServer;
class QstMessage;
class QtUiTestSlave;

class TestApplication : public QObject
{
    Q_OBJECT

public:

    enum State { NotStarted=0, Connected=1, ProcessingMessage=2, Disconnected=3 };
    explicit TestApplication( const QString &appId, AppManager *appManager = 0 );
    virtual ~TestApplication();
    bool connectToAut( int timeout );

    bool forwardMessage( QstMessage *msg, QtUiTestSlave *slave );
    void setPort(int port) { m_port = port; }
    int port() const { return m_port; }
    QString appId() const { return m_appId; }
    QString appName() const { return m_appName; }
    bool isConnected() const;
    State state() { return m_state; }
    void cleanup();

private slots:
    void onConnectionClosed();
    void onSignalReceived(const QVariantMap &sender, const QString &signal, const QVariantList &args);

signals:
    void finished(bool reusePort=true);

private:
    QString m_appId;
    QString m_appName;
    int m_port;
    QStringList m_env;
    QtUiTestMaster *m_connection;
    AppManager *m_appManager;
    State m_state;
};

class AppManager : public QObject
{
    Q_OBJECT

public:
    AppManager();
    virtual ~AppManager();

    void init(QtUiTestSlave *slave);
    bool forwardMessage( QstMessage *msg, QtUiTestSlave *slave );
    bool isConnected() const;
    int getFreePort() const;
    void setDefaultApplication(TestApplication*);
    QString currentApplication() const;
    QString homeApplication() const;
    QStringList getApplications(const QString & = QString());
    bool setApplication(const QString&);
    void purgeApplications();

    void appStandardOutput( TestApplication *app, QByteArray text );
    void appStandardError( TestApplication *app, QByteArray text );
    void postMessage( const QstMessage &msg );
    void testrunnerConnected( QtUiTestSlave *slave );
    void testrunnerDisconnected( QtUiTestSlave *slave );

    static AppManager* instance();

protected:
    bool connectApplication( int port, TestApplication *app, int timeout=30000 );
    void setApplication(TestApplication *app);

private slots:
    void onNewConnection();
    void onFinished(bool reusePort);

private:
    QList<QtUiTestSlave*> m_slaves;
    QPointer<TestApplication> m_currentApplication;

    // Default application is the system application, pre-started outside qtuitestd
    QPointer<TestApplication> m_defaultApplication;
    QMap<int, QPointer<TestApplication> > m_portMap;
    QList<int> m_pendingPorts;
    QMap<QString, QPointer<TestApplication> > m_appMap;
    QLocalServer *m_server;
};

#endif
