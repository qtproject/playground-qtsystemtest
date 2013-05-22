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

#include "qstconnection.h"
#include "qstprotocol.h"

#include <QtCore/QDebug>

const QLatin1String QstConnection::appIdProperty("_appid_");
const QLatin1String QstConnection::enumProperty("_enum_");
const QLatin1String QstConnection::signatureProperty("_signature_");
const QLatin1String QstConnection::valueProperty("_value_");
const QLatin1String QstConnection::widgetSpecifierProperty("widgetSpecifier");

QstConnection::QstConnection(QObject *parent)
    : QstProtocol(parent)
{
}

QstConnection::QstConnection(const QString &host, int port, QObject *parent)
    : QstProtocol(parent)
    , m_host(host)
    , m_port(port)
    , m_queryFailed(false)
{
}

void QstConnection::processMessage( QstMessage *msg )
{
    if (!msg) return;

    if (msg->event() == QLatin1String("APP_STARTED")) {
        emit appStarted(msg->context());
    } else if (msg->event() == QLatin1String("APP_FINISHED")) {
        emit appFinished(msg->context());
    } else {
        emit messageReceived(msg->event(), msg->context(), msg->toMap());
    }
}

bool QstConnection::connectToRemote()
{
    if (isConnected()) {
        qDebug() << Q_FUNC_INFO << "Already connected to "<< m_host;
        return true;
    }

    QTime t;
    t.start();
    while (t.elapsed() < 10000 && !isConnected()) {
        QstProtocol::connect(m_host, m_port);
        waitForConnected(2000);
    }

    if (!isConnected()) {
        qDebug() << Q_FUNC_INFO << "Could not connect to " << m_host << ':' << m_port;
/// FIXME: is this needed?
//         errorMsg = errorStr() ;
        return false;
    }

    return true;
}

QVariantMap QstConnection::sendQuery(const QString &event, const QVariantMap &params, const QVariantMap &widgetSpecifier)
{
    QstMessage message(event, params);
//    qDebug() << "message " << event << message.toMap();
    message[widgetSpecifierProperty] = widgetSpecifier[signatureProperty].toString();
    if (widgetSpecifier.contains(appIdProperty)) {
        qDebug() << " context set ";
        message.setContext(widgetSpecifier[appIdProperty].toString());
    }
    else if (params.contains(appIdProperty)) {
        message.setContext(params[appIdProperty].toString());
    }

    m_queryFailed = false;
    QstMessage reply;
    if (!sendMessage( message, reply, 10000 )) {
        // Unable to send - should fail test
        qDebug() << "Failed to send message";
        m_queryFailed = true;
        return reply.toMap();
    }

    return reply.toMap();
}

QVariantMap QstConnection::sendPositionQuery(const QString &event, const QVariantMap &params, const QPointF &position)
{
//    qDebug() << event << position;
    QVariantMap _params(params);
    _params["pos"] = position;
    return sendQuery(event, _params);
}


QVariantMap QstConnection::applicationData(const QString &appId)
{
    QstMessage message("applicationData");
    if (!appId.isEmpty()) {
        message.setContext(appId);
    }

    QVariantMap ret;
    QstMessage reply;
    if (!sendMessage(message, reply, 10000)) {
        return ret;
    }
    ret = reply["applicationdata"].toMap();
    return ret;
}

QImage QstConnection::grabImage(const QVariantMap &widgetSpecifier)
{
    QImage im;
    QVariantMap reply = sendQuery("grabImage", QVariantMap(), widgetSpecifier);
    if (!m_queryFailed) {
        im = reply["grabimage"].value<QImage>();
    }
    return im;
}

QVariantMap QstConnection::getObjects( const QVariantMap &widgetSpecifier )
{
    QVariantMap map;
    QVariantMap reply = sendQuery("getObjects", QVariantMap(), widgetSpecifier);
    if (!m_queryFailed) {
        map = reply["getobjects"].toMap();
    }
    return map;
}

QVariantMap QstConnection::getProperties( const QVariantMap &widgetSpecifier )
{
    QVariantMap ret;
    QVariantMap reply = sendQuery("getProperties", QVariantMap(), widgetSpecifier);
    if (!m_queryFailed) {
        ret = reply["getproperties"].toMap();
    }
    return ret;
}

QVariantList QstConnection::getMethods( const QVariantMap &widgetSpecifier )
{
    QVariantList ret;
    QVariantMap reply = sendQuery("getMethods", QVariantMap(), widgetSpecifier);
    if (!m_queryFailed) {
        ret = reply["getmethods"].toList();
    }
    return ret;
}

QVariantMap QstConnection::widgetFromSignature( const QString &signature, const QString &appId )
{
    QVariantMap ret;
    ret[signatureProperty] = signature;
    if (!appId.isEmpty())
        ret[appIdProperty] = appId;

    return ret;
}

QStringList QstConnection::getAppIds(const QString &name)
{
    QVariantMap params;
    if (!name.isEmpty()) {
        params["name"] = name;
    }
    QStringList ret;
    QVariantMap reply = sendQuery("getApplications", params);
    if (!m_queryFailed) {
        ret = reply["getapplications"].toStringList();
    }
    return ret;
}

QString QstConnection::currentAppId()
{
    QString ret;
    QVariantMap reply = sendQuery("currentApplication", QVariantMap());
    if (!m_queryFailed) {
        ret = reply["currentapplication"].toString();
    }
    return ret;
}

QString QstConnection::homeAppId()
{
    QString ret;
    QVariantMap reply = sendQuery("homeApplication", QVariantMap());
    if (!m_queryFailed) {
        ret = reply["homeapplication"].toString();
    }
    return ret;
}

void QstConnection::setAppId(const QString& appId)
{
    QstMessage message("setApplication");
    message.setContext(appId);

    QstMessage reply;
    sendMessage(message, reply, 10000);
}

QStringList QstConnection::findByProperties( const QVariantMap &searchValues, int flags )
{
    QVariantMap params;
    params["searchvalues"] = searchValues;
    params["flags"] = flags;

    QStringList ret;
    QVariantMap reply = sendQuery("queryProperties", params);

    if (!m_queryFailed) {
        QVariantList matches = reply["queryproperties"].toList();
        foreach (const QVariant &item, matches)
        {
            ret << item.toMap()[signatureProperty].toString();
        }
    }
    return ret;
}
