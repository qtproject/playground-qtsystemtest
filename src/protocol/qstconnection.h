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

#ifndef QSTCONNECTION_H
#define QSTCONNECTION_H

#include "qstprotocol.h"
#include <QtCore/QVariantMap>
#include <QtCore/QPointF>
#include <QImage>
#include <QStringList>

class QstConnection : public QstProtocol
{
    Q_OBJECT

public:

    enum QueryFlag
    {
        NoSort            = 0x01,
        NoScan            = 0x02,
        ScanInvisible     = 0x04,
        ScanDisabledItems = 0x08,
        IgnoreScanned     = 0x10
    };
    Q_DECLARE_FLAGS(QueryFlags, QueryFlag)

    QstConnection(QObject *parent = 0);
    explicit QstConnection(const QString & host, int port, QObject *parent = 0);

    static const QLatin1String appIdProperty;
    static const QLatin1String enumProperty;
    static const QLatin1String signatureProperty;
    static const QLatin1String valueProperty;
    static const QLatin1String widgetSpecifierProperty;


    QVariantMap applicationData(const QString &appId);
    QImage grabImage(const QVariantMap &widgetSpecifier);
    QVariantMap getObjects( const QVariantMap &widgetSpecifier );
    QVariantMap getProperties( const QVariantMap &widgetSpecifier );
    QVariantList getMethods( const QVariantMap &widgetSpecifier );
    QVariantMap widgetFromSignature( const QString &signature, const QString &appId );
    QStringList getAppIds(const QString & = QString());
    QString currentAppId();
    QString homeAppId();
    void setAppId(const QString& appId);
    QStringList findByProperties( const QVariantMap &searchValues, int flags );

signals:
    void appStarted(const QString &context);
    void appFinished(const QString &context);
    void messageReceived(const QString &event, const QString &context, const QVariantMap &data);

protected:
    virtual void processMessage( QstMessage *msg );

public slots:
    bool connectToRemote();
    QVariantMap sendQuery(const QString &event, const QVariantMap &params, const QVariantMap &widgetSpecifier = QVariantMap());
    QVariantMap sendPositionQuery(const QString &event, const QVariantMap &params, const QPointF &position);

private:
    QString m_host;
    int m_port;
    bool m_queryFailed;
};

#endif