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


/*
    This cpp file contains a number of socket related classes that are used heavily in QtUiTest.
    The classes are documented later in the file.
*/
#include "qstprotocol.h"

#ifdef QTUITEST_TARGET
#include <qtuitestnamespace.h>
#endif

#include <QCoreApplication>
#include <QString>
#include <QTimer>
#include <QUuid>
#include <QFileInfo>
#include <QDir>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QHostInfo>
#include <QPointer>

#include <QDebug>
#define qLog(A) if (1); else qDebug()

#if defined(Q_OS_WIN32) && !defined(Q_OS_TEMP)
# include <io.h>
#endif

static const int CONNECT_TIMEOUT = 20000;

static const quint32 TEST_MSG_SIGNATURE = 0xEDBAEDBA;
static const quint32 TEST_MSG_END       = 0xEDBAABDE;
static uint g_unique_id = 0;

bool waitForSignal(QObject* object, const char* signal, int timeout)
{
#ifdef QTUITEST_TARGET
    return QtUiTest::waitForSignal(object, signal, timeout);
#else
    QEventLoop loop;
    QTimer dummy;
    dummy.setInterval(1000);
    if (!QObject::connect(object, signal, &dummy, SLOT(start())))
        return false;
    if (!QObject::connect(object, signal, &loop, SLOT(quit())))
        return false;
    QTimer::singleShot(timeout, &loop, SLOT(quit()));
    loop.exec();
    return dummy.isActive();
#endif
}

void wait(int timeout)
{
#ifdef QTUITEST_TARGET
    QtUiTest::wait(timeout);
#else
    QEventLoop loop;
    QTimer::singleShot(timeout, &loop, SLOT(quit()));
    loop.exec();
#endif
}


/*
  \class QstMessage qstprotocol.h
    \inpublicgroup QtUiTestModule

  \brief The QstMessage class can be used for exchanging messages between separate
  processes. The class is never used directly but instead is used by QstProtocol.

  The class basically wraps a number of strings (i.e. 'event' and 'message') and binary data
  (i.e. a bytearray or a file) into a datastream that is sent to a peer using a socket
  connection. On the receiving side a QstMessage instance is decoding the datastream
  and performs a number of sanity checks to make it a bit more robost.

  A TCP connection 'should' be reliable but in exceptional cases bytes may be lost. This
  would result in the connection becoming useless because all future data reception would
  be out of sync. To solve this a 'resync' function is implemented that can re-sync the datastream
  by throwing away bytes until the data seems in sync again. The obvious downside is that
  at least one message will be lost.

  Message format is as follows:

  Field:              Length:        Remarks:

  Start signature     4              Fixed value - 0xEDBAEDBA
  Message state       1              See MessageState enum
  Message number      2
  Length              4              Length of event/context block
  Event:Context       Length         QString value (event and context)
  Data length         4              Length of following binary data
  File data           Data length    Binary data
  End signature       4              Fixed value - 0xEDBAABDE
*/

/*
   \internal

    Constructs a default (empty) message.
*/

QstMessage::QstMessage(const QString &event, QVariantMap const &map)
    : m_phase(0)
    , m_msg_id(0)
    , m_event(event)
    , m_state(MessageEmpty)
    , m_map(map)
{
}

/*
   \internal

   Copy constructor.
*/

QstMessage::QstMessage(const QstMessage &other)
    : m_phase(0)
    , m_msg_id(other.m_msg_id)
    , m_event(other.m_event)
    , m_context(other.m_context)
    , m_state(other.m_state)
    , m_map(other.m_map)
    , m_buffer(other.m_buffer)
{
}

/*
   \internal

    Destroys the message.
*/

QstMessage::~QstMessage()
{
}

/*
   \internal

   Assignment operator.
*/

QstMessage& QstMessage::operator=(const QstMessage &other)
{
    m_msg_id = other.m_msg_id;
    m_event = other.m_event;
    m_context = other.m_context;
    m_state = other.m_state;
    m_map = other.m_map;
    m_buffer = other.m_buffer;

    return *this;
}

QVariant &QstMessage::operator[](const QString &key)
{
    unpack();
    return m_map[key.toLower()];
}

const QVariant QstMessage::operator[](const QString &key) const
{
    unpack();
    return m_map[key.toLower()];
}

bool QstMessage::contains(const QString &key) const
{
    unpack();
    return m_map.contains(key.toLower());
}

QList<QString> QstMessage::keys() const
{
    unpack();
    return m_map.keys();
}

QString QstMessage::toString() const
{
    QString ret;
    if (!isUnpacked()) {
        ret = QLatin1String("(packed)\n");
        return ret;
    }

    QVariantMap::const_iterator i = m_map.constBegin();
    while (i != m_map.constEnd()) {
        if (!i.value().isValid()) {
            i++;
            continue;
        }
        ret += i.key() + QLatin1String(": ");
        if (i.value().canConvert<QStringList>())
            ret += QLatin1String("'") + i.value().toStringList().join(QLatin1String("','")) + QLatin1String("'");
        else if (i.value().canConvert<QString>())
            ret += QLatin1String("'") + i.value().toString() + QLatin1String("'");
        else
            ret += QLatin1String("(data)");
        ret += QLatin1Char('\n');
        ++i;
    }

    if (ret.endsWith(QLatin1Char('\n'))) ret.chop(1);
    return ret;
}

QVariantMap QstMessage::toMap() const
{
    return m_map;
}

/*
  \internal
    Returns the event that was received.
*/

QString QstMessage::event() const
{
    return m_event;
}

void QstMessage::setEvent(const QString &event)
{
    m_event = event;
}

/*
  \internal
    Returns the message number.
*/

quint16 QstMessage::msgId() const
{
    return m_msg_id;
}

bool QstMessage::isNull() const
{
    return m_map.isEmpty();
}

bool QstMessage::isUnpacked() const
{
    return m_buffer.isEmpty();
}

void QstMessage::unpack() const
{
    if (isUnpacked()) return;

    QDataStream s(&m_buffer, QIODevice::ReadOnly);
    s.setVersion(QDataStream::Qt_4_6);
    s >> m_map;
    m_buffer.clear();
}


/*
  \class QstProtocol qstprotocol.h
    \inpublicgroup QtUiTestModule

  \brief The QstProtocol class can be used for exchanging messages between separate
  processes.

  It is intended to be fast but at the same time ultra reliable and robust communication protocol.

  The main function that is used on the sending side is:
  \code
  myConnection.postMessage( "My-Event", "My-Message", ptr_to_my_data );
  \endcode

  On the receiving side the main function is a re-implemented 'processMessage':
  \code
  void MyTestConnection::processMessage( QstMessage *msg )
  {
    if (msg->event() == "My-Event") {
        print( msg->message() );
    }
  }
  \endcode
*/

#include <stdio.h>

QstProtocol::QstProtocol(QObject *parent)
    : QTcpSocket(parent)
    , tx_msg_id(1)
    , port(0)
    , onData_busy(false)
    , enable_reconnect(false)
    , reconnect_interval(10000)
    , last_data_received(false)
    , connection_valid(false)
    , debug_on(false)
    , m_auto_unpack(true)
{
    static int id1 = qRegisterMetaType<QstMessage>(); Q_UNUSED(id1);
    static int id2 = qRegisterMetaType<QstMessage*>(); Q_UNUSED(id2);
    static int id3 = qRegisterMetaType<const QstMessage*>(); Q_UNUSED(id3);

    unique_id = QString::number(++g_unique_id);
    if (debug_on) {
        qDebug() << QString("%1 QstProtocol::QstProtocol()").arg(uniqueId()).toLatin1();
    }
    cur_message = 0;
    rx_busy = false;

    QObject::connect( &connect_timer, SIGNAL(timeout()), this, SLOT(connectTimeout()), Qt::DirectConnection );

    QObject::connect( this,SIGNAL(connected()),this,SLOT(onSocketConnected()), Qt::DirectConnection );
    QObject::connect( this,SIGNAL(disconnected()),this,SLOT(onSocketClosed()), Qt::DirectConnection );
    QObject::connect( this,SIGNAL(readyRead()),this,SLOT(onData()), Qt::DirectConnection );

    // initialize. Any time is better than no time.
    rx_timer.start();
}

/*!
    Destructs the instance of QstProtocol.
*/

QstProtocol::~QstProtocol()
{
    if (debug_on) {
        qDebug() <<  QString("%1 QstProtocol::~QstProtocol()").arg(uniqueId()).toLatin1();
    }
    enableReconnect( false, 0 );

    // anything that is still in the tx buffer gets lost
    abort();
    close();

    while (send_msg_replies.count() > 0)
        delete send_msg_replies.takeFirst();
    while (unhandled_msg.count() > 0)
        delete unhandled_msg.takeFirst();
}

void QstProtocol::setSocket( int socket )
{
    if (debug_on) {
        qDebug() << ( QString("%1 QstProtocol::setSocket(socket=%2)").
                            arg(uniqueId()).
                            arg(socket).toLatin1());
    }
    setSocketDescriptor( socket );

    rx_timer.start();
    testConnection();
}

void QstProtocol::enableReconnect( bool enable, uint reconnectInterval )
{
    if (debug_on) {
        qDebug() << ( QString("%1 QstProtocol::enableReconnect( enable=%2, interval=%3)").
                            arg(uniqueId()).
                            arg(enable).
                            arg(reconnectInterval).toLatin1());
    }
    enable_reconnect = enable;
    reconnect_interval = reconnectInterval;
}

/*!
    Opens a socket connection to the specified \a hostname and \a port.

    After a connection is successfully opened the instance will listen for and process
    incoming commands received from the remote host.
*/
void QstProtocol::connect( const QString& hostname, int port )
{
    if (state() == ConnectedState) {
        if (hostname == this->host && port == this->port)
            return;
        disconnect();
    }

    if (debug_on) {
        qDebug() << ( QString("%1 QstProtocol::connect(%2:%3)").
                            arg(uniqueId()).
                            arg(hostname).
                            arg(port).toLatin1());
    }

    rx_timer.start();

    this->host = hostname;
    this->port = port;

    reconnect();
}

void QstProtocol::disconnect( bool disableReconnect )
{
    if (state() == ConnectedState) {
        if (debug_on) {
            qDebug() << ( QString("%1 QstProtocol::disconnect(disableReconnect=%2)").
                                arg(uniqueId()).
                                arg(disableReconnect).toLatin1());
        }
        // be polite and tell the other side we are closing
        postMessage( QstMessage("QTEST_CLOSING_CONNECTION") );

        // we are closing ourselves, so we don't want to reconnect
        if (disableReconnect) enable_reconnect = false;

        onSocketClosed();
    }
}

bool QstProtocol::isConnected()
{
    return ((state() == ConnectedState) && connection_valid);
}

bool QstProtocol::waitForConnected( int timeout )
{
    QTime t;
    t.start();
    bool ok = false;
    QTime t2;
    t2.start();
    while (t.elapsed() < timeout && !ok) {
        ok = QTcpSocket::waitForConnected(timeout);
        if (!ok) {
            wait(100);
            if (t2.elapsed() > 1000) {
                t2.start();
                reconnect();
            }
        }
    }

    if (ok) {
        if (debug_on) {
            qDebug() << QString("%1 QstProtocol::waitForConnected(%2, %3) ...")
                        .arg(uniqueId()).arg(timeout).arg(stateStr()).toLatin1();
        }
        t2.start();
        while (t.elapsed() < timeout && !isConnected()) {
            wait(10);
            if (!isConnected() && t2.elapsed() > 500) {
                postMessage( QstMessage(QLatin1String("QTEST_NEW_CONNECTION")) );
                t2.start();
            }
        }
        ok = isConnected();
    }

    if (debug_on) {
        qDebug() << QString("%1 QstProtocol::waitForConnected() ... %2 (%3 ms)").arg(uniqueId()).arg(ok ? "OK" : "FAILED" ).arg(t.elapsed()).toLatin1();
        if (!ok) qDebug() << errorStr();
    }
    return ok;
}

/*!
   \internal
    Posts (e.g. non blocking) an \a event, \a message and contents of \a fileName to the remote host.
*/

uint QstProtocol::postMessage(const QstMessage &message )
{
    if (debug_on && message.event() != QLatin1String("PING") && message.event() != QLatin1String("PONG")) {
        qDebug() << ( QString("%1 QstProtocol::postMessage(%2)").
                            arg(uniqueId()).
                            arg(message.event()).toLatin1());
    }
    if (state() != ConnectedState)
        return 0;
    QstMessage msg(message);
    msg.m_msg_id = tx_msg_id++;
    send( msg );
    return msg.m_msg_id;
}

void QstProtocol::onReplyReceived( QstMessage* /*reply*/ )
{
}

/*!
   \internal
    Sends an \a event, \a message and \a data to the remote host and waits for up to
    \a timeout milliseconds for a reply.  If a reply is received, the reply's message
    string is placed in \a reply.
*/
bool QstProtocol::sendMessage( const QstMessage &message, QstMessage &reply, int timeout )
{
    QstMessage msg(message);
    QPointer<QstProtocol> safeThis(this);
    bool safeDebugOn(debug_on);
    QString safeUniqueId(uniqueId());

    last_send_cmd = message.event();

    if (state() == ConnectingState) {
        wait(4000);
    }

    if (state() == ConnectedState) {
        msg.m_msg_id = tx_msg_id++;

        if (debug_on) {
            qDebug() << ( QString("%1 QstProtocol::sendMessage(%2) msgid=%3)").
                                arg(uniqueId()).
                                arg(msg.event()).
                                arg(msg.msgId()).
                                toLatin1());
        }

        send( msg );

        QTime t1;
        t1.start();
        QTime t2;
        t2.start();
        bool first_time = true;
        while ( (state() == ConnectedState) &&
               (timeout < 0 || t1.elapsed() < timeout) ) {

            if (debug_on) {
                if (first_time || t2.elapsed() > 1000) {
                    qDebug() << ( QString("%1 QstProtocol::sendMessage(%2) ... waiting for reply").
                                        arg(uniqueId()).
                                        arg(message.event()).toLatin1());
                    t2.start();
                    first_time = false;
                }
            }

            waitForSignal(this, SIGNAL(replyReceived()), 500);

            if (!safeThis) {
                if (safeDebugOn) {
                    qDebug() << ( QString("%1 QstProtocol::sendMessage(%2) ... object deleted unexpectedly").
                                        arg(safeUniqueId).
                                        arg(message.event()).toLatin1());
                }
                reply[QLatin1String("status")] = "ERROR: Connection was terminated unexpectedly. This may be caused by an application crash.";
                reply[QLatin1String("_q_inResponseTo")] = QString("%1\n%2").arg(message.event()).arg(message.toString());
                return false;
            } else {
                if (send_msg_replies.count() > 0) {
                    if (debug_on) {
                        qDebug() << ( QString("%1 QstProtocol::sendMessage(%2) ... check replies").
                                            arg(uniqueId()).
                                            arg(message.event()).toLatin1());
                    }
                    for (int i=0; i<send_msg_replies.size(); i++) {
                        QstMessage * possible_reply = send_msg_replies.at(i);
                        if (possible_reply && possible_reply->m_msg_id == msg.m_msg_id) {
                            reply = *possible_reply;
                            delete send_msg_replies.takeAt( i );
                            if (debug_on) {
                                qDebug() << ( QString("%1 QstProtocol::sendMessage(%2) ... reply received").
                                                    arg(uniqueId()).
                                                    arg(message.event()).toLatin1());
                            }

                            onReplyReceived(&reply);
                            return true;
                        }
                    }
                }
            }
        }
        if (state() != ConnectedState) {
            reply[QLatin1String("status")] = "ERROR: Connection lost. This is likely caused by a crash in the Application Under Test.";
            reply[QLatin1String("_q_inResponseTo")] = QString("%1\n%2").arg(message.event()).arg(message.toString());
        } else {
            qDebug() << "ERROR-REPLY-TIMEOUT: " << t1.elapsed() << t2.elapsed();
            reply[QLatin1String("status")] = QLatin1String("ERROR_REPLY_TIMEOUT");
            reply[QLatin1String("_q_inResponseTo")] = QString("%1\n%2").arg(message.event()).arg(message.toString());
        }
        reply[QLatin1String("location")] = QString("%1:%2").arg(__FILE__).arg(__LINE__);
    } else {
        reply[QLatin1String("status")] = QLatin1String("ERROR_NO_CONNECTION");
        reply[QLatin1String("_q_inResponseTo")] = QString("%1\n%2").arg(message.event()).arg(message.toString());
        reply[QLatin1String("location")] = QString("%1:%2").arg(__FILE__).arg(__LINE__);
    }

    if (debug_on) {
        qDebug() << ( QString("%1 QstProtocol::sendMessage(%2) ... done. Status: %3").
                            arg(uniqueId()).
                            arg(message.event()).arg(reply[QLatin1String("status")].toString()).toLatin1());
    }

    return false;
}

/*!
    Send the string \a result as a reply to \a originalMsg.
*/

void QstProtocol::replyMessage( const QstMessage *originalMsg, const QstMessage &message )
{
    if (debug_on) {
        qDebug() << ( QString("%1 QstProtocol::replyMessage(%2)").
                            arg(uniqueId()).
                            arg(originalMsg->event()).toLatin1());
    }

    QstMessage msg(message);
    msg.m_msg_id = originalMsg->msgId();
    msg.m_event = QLatin1String("@REPLY@");
    if (originalMsg->state() == QstMessage::MessageOldVersion) {
        msg["status"] = QLatin1String(
        "\n**********************************************"
        "\nThe version of qtuitestrunner you are using is"
        "\nincompatible with this version of QtUiTest!"
        "\n**********************************************\n");
    }
    send( msg );
}

bool QstProtocol::lastDataReceived()
{
    return last_data_received;
}

QString QstProtocol::errorStr()
{
    QString S = QLatin1String("Connection error: ");
    switch (error()) {
        case ConnectionRefusedError: S += QString("Connection attempt to %1:%2 failed").arg(host).arg(port).toLatin1(); break;
        case HostNotFoundError: S += QString("Host %1:%2 not found").arg(host).arg(port).toLatin1(); break;
        case RemoteHostClosedError: S += QLatin1String("RemoteHostClosedError"); break;
        case SocketAccessError: S += QLatin1String("SocketAccessError"); break;
        case SocketResourceError: S += QLatin1String("SocketResourceError"); break;
        case SocketTimeoutError: S += QLatin1String("SocketTimeoutError"); break;
        case DatagramTooLargeError: S += QLatin1String("DatagramTooLargeError"); break;
        case NetworkError: S += QLatin1String("NetworkError"); break;
        case AddressInUseError: S += QLatin1String("AddressInUseError"); break;
        case SocketAddressNotAvailableError: S += QLatin1String("SocketAddressNotAvailableError"); break;
        case UnsupportedSocketOperationError: S += QLatin1String("UnsupportedSocketOperationError"); break;
        case UnknownSocketError: S += QLatin1String("UnknownSocketError"); break;
        default: S += QLatin1String("Unknown error");
    }

    return S;
}

QString QstProtocol::stateStr()
{
    switch (state()) {
        case QAbstractSocket::UnconnectedState: return QLatin1String("UnconnectedState");
        case QAbstractSocket::HostLookupState: return QLatin1String("HostLookupState");
        case QAbstractSocket::ConnectingState: return QLatin1String("ConnectingState");
        case QAbstractSocket::ConnectedState: return QLatin1String("ConnectedState");
        case QAbstractSocket::BoundState: return QLatin1String("BoundState");
        case QAbstractSocket::ClosingState: return QLatin1String("ClosingState");
        case QAbstractSocket::ListeningState: return QLatin1String("ListeningState");
    }
    return QLatin1String("Unknown state");
}

void QstProtocol::onConnectionFailed( const QString &reason )
{
    emit connectionFailed( this, reason );
}

void QstProtocol::testConnection()
{
    if (debug_on) {
        qDebug() <<  QString("%1 QstProtocol::testConnection()").arg(uniqueId()).toLatin1();
    }

    while (send_msg_replies.count() > 0)
        delete send_msg_replies.takeFirst();

    postMessage( QstMessage(QLatin1String("QTEST_NEW_CONNECTION")) );
}

void QstProtocol::send( const QstMessage &message )
{
    QByteArray data;

    if (!message.isUnpacked()) {
        data = message.m_buffer;
    } else if (!message.m_map.isEmpty()) {
        QDataStream s(&data, QIODevice::WriteOnly);
        s.setVersion(QDataStream::Qt_4_6);
        s << message.m_map;
    }

    QDataStream tmp(this);
    tmp.setVersion(QDataStream::Qt_4_6);
    sendPreamble(&tmp, message);

    // phase 2
    quint32 len = data.count();
    tmp << len;
    if (len > 0) {
        // phase 3
        tmp.writeRawData( data.data(), (int)len );
    }

    // phase 4
    tmp << TEST_MSG_END;

    flush();    // Force socket to send data now
}

void QstProtocol::sendPreamble( QDataStream *ds, const QstMessage &message)
{
    // phase 0
    *ds << TEST_MSG_SIGNATURE;
    *ds << static_cast<quint8>(message.state());
    *ds << message.msgId();

    // phase 1
    QString event;
    if (message.context().isEmpty()) {
        event = message.event();
    } else {
        event = message.event() + QChar(':') + message.context();
    }

    quint32 len = (event.length() *2) + 4;
    *ds << len;
    *ds << event;
}

bool QstProtocol::receive( QstMessage *msg, bool &syncError )
{
    syncError = false;

    QDataStream stream(this);
    stream.setVersion(QDataStream::Qt_4_6);

    quint8 msgState;
    if (msg->m_phase == uint(0)) {
        msg->m_len = 0;
        quint32 sig;
        if (bytesAvailable() >= sizeof( sig ) + sizeof( msgState ) + sizeof( msg->m_msg_id ) + sizeof( msg->m_len )) {
            stream >> sig;
            if (sig != TEST_MSG_SIGNATURE) {
                qWarning() << QString("QstMessage::receive(), Invalid start signature (0x%1)").arg(sig,8,16).toLatin1();
                syncError = true;
                return false;
            } else {
                stream >> msgState;
                msg->m_state = static_cast<QstMessage::MessageState>(msgState);
                stream >> msg->m_msg_id;
                stream >> msg->m_len;
                msg->m_phase++;
            }
        }
    }

    if (msg->m_phase == uint(1)) {
        if (bytesAvailable() >= msg->m_len) {
            QString event;
            stream >> event;
            int i = event.indexOf(QLatin1Char(':'));
            if (i == -1) {
                msg->m_event = event;
            } else {
                msg->m_event = event.left(i);
                msg->m_context = event.mid(i+1);
            }
            msg->m_phase++;
        }
    }

    if (msg->m_phase == uint(2)) {
        if (bytesAvailable() >= sizeof( msg->m_len )) {
            stream >> msg->m_len;
            msg->m_phase++;
        }
    }

    if (msg->m_phase == uint(3)) {
        if (msg->m_len > 0) {
            QByteArray buf;
            quint32 len = msg->m_len;
            uint bytes_available = bytesAvailable();
            if (bytes_available < len)
                len = bytes_available;
            buf.resize( len );
            stream.readRawData( buf.data(), len );
            msg->m_buffer.append(buf);
            msg->m_len -= len;

            if (msg->m_len == 0) {
                // received OK
                msg->m_phase++;
            } else {
                // waiting for more data
                return false;
            }

        } else {
            msg->m_phase++;
        }
    }

    if (msg->m_phase == uint(4)) {
        quint32 id2;
        if (bytesAvailable() >= sizeof( id2 )) {
            stream >> id2;
            msg->m_phase = 0;
            if (id2 != TEST_MSG_END) {
                qWarning() << QString("QstMessage::receive(), Invalid end signature (0x%2)").arg(id2,8,16).toLatin1();
                syncError = true;
                return false;
            } else {
                return true;
            }
        }
    }
    return false;
}

bool QstProtocol::rxBusy()
{
    return rx_busy;
}

/*!
    Reads the remote control connection and responds to received commands.
*/

void QstProtocol::onData()
{
    if (onData_busy) return;
    onData_busy = true;

    int sync_error_count = 0;
    bool msg_received = true;
    while (msg_received && bytesAvailable() > 0) {
        msg_received = false;
        // set the time to now :-)
        rx_timer.start();

        bool sync_error;
        if (cur_message == 0)
            cur_message = new QstMessage();

        if (receive( cur_message, sync_error )) {

            msg_received = true;
            QString last_event = cur_message->event();

            if (m_auto_unpack) {
                cur_message->unpack();
            }

            if (debug_on) {
                qDebug() << ( QString("%1 QstProtocol::onData(%2) msgid = %3").
                                  arg(uniqueId()).
                                  arg(last_event).
                                  arg(cur_message->m_msg_id).
                                  toLatin1());
            }

            // We received a full message
            if (last_event == QLatin1String("@REPLY@")) {
                // add the reply to a list
                send_msg_replies.append( cur_message );
                int id = cur_message->m_msg_id;
                QstMessage *received_message(cur_message);
                 // and make sure we create a new one
                cur_message = 0;
                emit replyReceived( id, received_message );
            } else if (last_event == QLatin1String("QTEST_NEW_CONNECTION")) {
                // Acknowledge the other side we can receive data
                postMessage( QstMessage(QLatin1String("QTEST_ACK_CONNECTION")) );
            } else if (last_event == QLatin1String("QTEST_ACK_CONNECTION")) {
                // we don't assume we have a connection until both sides
                // have actually received data from the other side
                connection_valid = true;
                onConnected();
                connect_timer.stop();
            } else if (last_event == QLatin1String("QTEST_CLOSING_CONNECTION")) {
                last_data_received = true;
                QTimer::singleShot( 0, this, SLOT(onSocketClosed()));
            } else if (last_event == QLatin1String("PONG")) {
                // Do nothing
            } else if (last_event == QLatin1String("TEST")) {
                if (!debug_on) {
                    // don't show the same information twice
                    qDebug() <<  QString("%1 Test message received").arg(uniqueId()).toLatin1();
                }
            } else if (last_event == QLatin1String("PING")) {
                postMessage( QstMessage(QLatin1String("PONG")) );
            } else {
                // add the msg to a list
                unhandled_msg.append( cur_message );
                cur_message = 0;
                QTimer::singleShot(0, this, SLOT(processMessages()));
            }

            delete cur_message;
            cur_message = 0;
        } else {
            // We didn't receive a full message
            if (sync_error) {
                sync_error_count++;
                if (sync_error_count > 10)
                    return;
                // receiving garbage messages - nothing can be done but closing the connection and try again.
                delete cur_message;
                cur_message = 0;
                disconnect(!enable_reconnect);
                reconnect();
            }
            // else we are waiting on more fragments to arrive
        }
    }
    onData_busy = false;
}

void QstProtocol::processMessages()
{
    while (!rx_busy && unhandled_msg.count() > 0) {
        QstMessage *tmp = unhandled_msg.takeFirst();
        if (tmp) {
            rx_busy = true;
            processMessage( tmp );
            delete tmp;
            rx_busy = false;
        }
    }
}

/*!
    Signals the instance that the other side has closed the connection.
*/
void QstProtocol::onSocketClosed()
{
    if (debug_on) {
        qDebug() << QString("%1 QstProtocol::onSocketClosed()").arg(uniqueId()).toLatin1();
    }

    // anything that is still in the tx buffer gets lost
    abort();

    close();

    connection_valid = false;

    // if the close was spontaneous and we want to keep the connection alive, we try to reconnect
    if (enable_reconnect) {
        if (debug_on) {
            qDebug() <<
                QString("%1 QstProtocol::onSocketClosed() singleshot reconnect in .5 seconds").arg(uniqueId()).toLatin1();
        }
        QTimer::singleShot(500,this,SLOT(reconnect()));
    }

    // tell the world we are closed
    QTimer::singleShot(0, this, SLOT(emitConnectionClosed()));
}

/*!
    Signals the instance that a connection is established with a remote control host.
*/
void QstProtocol::onSocketConnected()
{
    connect_timer.stop();
    if (debug_on) {
        qDebug() << QString("%1 QstProtocol::onSocketConnected()").arg(uniqueId()).toLatin1();
    }
    testConnection();
}

void QstProtocol::reconnect()
{
    if (!host.isEmpty() && state() != ConnectedState) {
        if (debug_on) {
            qDebug() << QString("%1 QstProtocol::reconnect(%2)").arg(uniqueId()).arg(stateStr()).toLatin1();
        }

        connect_timer.stop();
        connect_timer.start( CONNECT_TIMEOUT );

        // if we are trying to connect to the local machine, always use 127.0.0.1
        // (and avoid the need for dns)
        QString hostname = QHostInfo::localHostName().toUpper();
        if (hostname == host.toUpper() || hostname.startsWith( host.toUpper() + "." ))
            host = QLatin1String("127.0.0.1");

        close();
        wait(100);
        if (state() == ConnectingState) {
            // Will just get a warning, retry later
            return;
        }
        connectToHost( host, port );
    } else {
        if (host.isEmpty()) {
            qWarning( "QstProtocol::reconnect() FAILED, no host specified" );
            enable_reconnect = false;
        }
    }
}

void QstProtocol::connectTimeout()
{
    if (debug_on) {
        qDebug() << QString("%1 QstProtocol::connectTimeout()").arg(uniqueId()).toLatin1();
    }

    connect_timer.stop();
    if (enable_reconnect) {
        reconnect();
    } else {
        onConnectionFailed( errorStr() );
    }
}

void QstProtocol::emitConnectionClosed()
{
    if (debug_on) {
        qDebug() << QString("%1 QstProtocol::emitConnectionClosed()").arg(uniqueId()).toLatin1();
    }

    // force sendMessage to quit
    emit replyReceived();
    emit connectionClosed( this );
}


QString QstProtocol::uniqueId() const
{
    return QString("%1 %2").arg(unique_id).arg(qApp->applicationName());
}

void QstProtocol::enableDebug( bool debugOn )
{
    debug_on = debugOn;
    qDebug() << QString("Debugging is switched %1 for Test Protocol %2").arg(debugOn ? "ON" : "OFF").arg(uniqueId()).toLatin1() ;
}

void QstProtocol::disableDebug()
{
    debug_on = false;
    qDebug() << QString("Debugging is switched %1 for Test Protocol %2").arg(debug_on ? "ON" : "OFF").arg(uniqueId()).toLatin1() ;
}
