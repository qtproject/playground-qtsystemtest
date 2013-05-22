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


#include "qtuitestconnectionmanager_p.h"

#include <QCoreApplication>
#include <QObject>
#include <QThread>

// from qobject_p.h
struct QSignalSpyCallbackSet
{
    typedef void (*BeginCallback)(QObject*,int,void**);
    typedef void (*EndCallback)(QObject*,int);
    BeginCallback signal_begin_callback,
                  slot_begin_callback;
    EndCallback signal_end_callback,
                slot_end_callback;
};
extern void Q_CORE_EXPORT qt_register_signal_spy_callbacks(QSignalSpyCallbackSet const&);
void qtuitest_signal_begin(QObject*,int,void**);

Q_GLOBAL_STATIC(QtUiTestConnectionManager,_q_qtUiTestConnectionManager);

QtUiTestConnectionManager::~QtUiTestConnectionManager()
{
    unregisterCallback();
}

QtUiTestConnectionManager* QtUiTestConnectionManager::instance()
{
    return _q_qtUiTestConnectionManager();
}

void qtuitest_signal_begin(QObject* sender, int signal, void** argv)
{
    QtUiTestConnectionManager* cm = QtUiTestConnectionManager::instance();

    // Should only ever be null when the application is shutting down, but in that case,
    // this callback should already have been unregistered.
    Q_ASSERT(cm);

    // During the destructor it isn't safe to check which thread we are.
    // We'll just ignore all signals during this time.
    if (QCoreApplication::closingDown()) return;

    // Connections are only supported in the main thread.
    if (QThread::currentThread() != QCoreApplication::instance()->thread()) return;

    cm->activateConnections(sender,signal,argv);
}

void QtUiTestConnectionManager::connect(const QObject* sender, int senderMethod,
        const QObject* receiver, int receiverMethod)
{
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    registerCallback();

    Connection c = {
        const_cast<QObject*>(sender),   senderMethod,
        const_cast<QObject*>(receiver), receiverMethod
    };
    m_connections << c;
}

bool QtUiTestConnectionManager::disconnect(const QObject* sender, int senderMethod,
        const QObject* receiver, int receiverMethod)
{
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    bool ret = false;

    QList<Connection>::iterator iter = m_connections.begin();
    while (iter != m_connections.end())
    {
        bool remove = true;

        if (sender && (sender != iter->sender))                                 remove = false;
        if ((senderMethod != -1) && (senderMethod != iter->senderMethod))       remove = false;
        if (receiver && (receiver != iter->receiver))                           remove = false;
        if ((receiverMethod != -1) && (receiverMethod != iter->receiverMethod)) remove = false;

        if (remove) {
            ret = true;
            iter = m_connections.erase(iter);
        }
        else {
            ++iter;
        }
    }

    unregisterCallback();

    return ret;
}

void QtUiTestConnectionManager::registerCallback()
{
    // If this is the first connect, then register the callback function
    if (m_connections.isEmpty()) {
        QSignalSpyCallbackSet callbacks = { qtuitest_signal_begin, 0, 0, 0 };
        qt_register_signal_spy_callbacks(callbacks);
    }
}

void QtUiTestConnectionManager::unregisterCallback()
{
    // If everything has been disconnected, remove the callback function
    if (m_connections.isEmpty()) {
        QSignalSpyCallbackSet callbacks = { 0, 0, 0, 0 };
        qt_register_signal_spy_callbacks(callbacks);
    }
}

void QtUiTestConnectionManager::activateConnections(QObject* sender, int senderMethod, void** argv)
{
    static const int destroyedMethod = QObject::staticMetaObject.indexOfSignal("destroyed(QObject*)");
    const bool destroyed = (senderMethod == destroyedMethod);

    // Find all of the connections we need to activate.
    QList<Connection> toActivate;
    QList<Connection>::iterator iter = m_connections.begin();
    while (iter != m_connections.end())
    {
        if (sender == iter->sender && senderMethod == iter->senderMethod)
            toActivate << *iter;

        // Remove this connection if either the sender or receiver is being destroyed
        if (destroyed && (sender == iter->sender || sender == iter->receiver)) {
            iter = m_connections.erase(iter);
            unregisterCallback();
        }
        else
            ++iter;
    }

    foreach (Connection const& c, toActivate)
    { if (c.receiver) c.receiver->qt_metacall(QMetaObject::InvokeMetaMethod, c.receiverMethod, argv); }
}

