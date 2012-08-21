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


#ifndef QTESTPROTOCOL_P_H
#define QTESTPROTOCOL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtUiTest API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QTimer>
#include <QTime>
#include <QObject>
#include <QString>
#include <QFile>
#include <QVariant>
#include <QMap>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>

#include "qstglobal.h"

#define REMOTE_CONNECT_ERROR 99

#define MESSAGE_ERROR(message, str)  (\
    message.setState(QstMessage::MessageError),\
    message["status"] = str,\
    message)

class QstMessage
{
public:
    enum MessageState {
        MessageEmpty   = 0,
        MessageSuccess = 1,
        MessageError   = 2,
        MessageOldVersion = 3
    };

    QstMessage(const QString &event = QString(), QVariantMap const &map = QVariantMap() );
    QstMessage(const QstMessage &other);
    virtual ~QstMessage();

    QstMessage& operator=(const QstMessage &other);

    QString event() const;
    void setEvent(const QString &event);
    quint16 msgId() const;

    const QVariant operator[](const QString &key) const;
    QVariant &operator[](const QString &key);
    bool contains(const QString &key) const;
    QList<QString> keys() const;

    QString toString() const;
    QVariantMap toMap() const;

    bool isNull() const;
    bool isUnpacked() const;
    void unpack() const;
    MessageState state() const { return m_state; }
    void setState(MessageState state) { m_state = state; }

    QString context() const { return m_context; }
    void setContext(const QString &context) { m_context = context; }

protected:
    uint          m_phase;
    quint32       m_len;

    quint16       m_msg_id;
    QString       m_event;
    QString       m_context;
    MessageState  m_state;

    mutable QVariantMap   m_map;
    mutable QByteArray    m_buffer;

    friend class QstProtocol;
};
Q_DECLARE_METATYPE( QstMessage )
Q_DECLARE_METATYPE( QstMessage* )
Q_DECLARE_METATYPE( const QstMessage* )

class QstProtocol : public QTcpSocket
{
    Q_OBJECT

public:
    QstProtocol(QObject *parent = 0);
    virtual ~QstProtocol();

    virtual void setSocket( int socket );

    void enableReconnect( bool enable, uint reconnectInterval = 5000 );

    void connect( const QString& hostname, int port );
    void disconnect( bool disableReconnect = true );
    bool isConnected();
    virtual bool waitForConnected( int timeout = 10000 );

    virtual uint postMessage( const QstMessage &message );

    virtual bool sendMessage( const QstMessage &message, QstMessage &reply, int timeout );
    virtual void replyMessage( const QstMessage *originalMsg, const QstMessage &message );

    bool lastDataReceived();
    bool rxBusy();
    virtual void onReplyReceived( QstMessage *reply );

    QString errorStr();
    QString stateStr();

    virtual void onConnectionFailed( const QString &reason );
    virtual void onConnected() {}; // re-implement in derived class

    QString uniqueId() const;
    void enableDebug( bool debugOn );
    void setAutoUnpack( bool autoUnpack ) { m_auto_unpack = autoUnpack; }

public slots:
    void reconnect();
    void disableDebug();

protected:
    virtual void processMessage( QstMessage *msg ) = 0;

    void send( const QstMessage &message );
    void sendPreamble( QDataStream *ds, const QstMessage &message );
    bool receive( QstMessage *msg, bool &syncError );

signals:
    void connectionClosed( QstProtocol *socket );
    void connectionFailed( QstProtocol *socket, const QString &reason );
    void replyReceived(int id = -1, const QstMessage *message = 0);
    void replyConfirmed();

protected slots:
    void onData();
    void onSocketConnected();
    void onSocketClosed();
    void connectTimeout();
    void emitConnectionClosed();
    void processMessages();
    void testConnection();

private:
    quint16         tx_msg_id;
    QString         host;
    int             port;
    bool            onData_busy;
    bool            enable_reconnect;
    uint            reconnect_interval;
    QTimer          connect_timer;
    QTime           rx_timer;
    bool            last_data_received;
    bool            rx_busy;
    bool            connection_valid;

    QList<QstMessage*> send_msg_replies;
    QList<QstMessage*> unhandled_msg;
    QstMessage   *cur_message;

    QString         last_send_cmd;
    QString         unique_id;
    bool            debug_on;
    bool            m_auto_unpack;
};

#endif
