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


#include <qtuitestdsettings.h>
#include <QCoreApplication>
#include <QStringList>
#include <QDebug>

#define DEFAULT_PORT 5656
#define DEFAULT_APPPORTMIN 5658
#define DEFAULT_APPPORTMAX 5699

QtUiTestDSettings::QtUiTestDSettings() :
    m_debug(false),
    m_port(DEFAULT_PORT),
    m_portMax(DEFAULT_PORT),
    m_appPortMin(DEFAULT_APPPORTMIN),
    m_appPortMax(DEFAULT_APPPORTMAX),
    m_prestartPort(0)
{
    const QStringList arguments = QCoreApplication::arguments();
    int args = arguments.size();
    QStringList deviceConfig;

    for (int i=1; i<args; ++i) {
        QString arg = arguments.at(i);
        if (QString("-help").indexOf(arg) == 0) {
            qWarning() << QString(
            "%1 - QtUITest daemon\n\n"
            "    -help              : Display this help text and exit\n"
            "    -debug             : Display debugging information\n\n"
            "    -port port         : Specify port to listen on (default: %2)\n"
            "    -port start,end    : Specify port range to listen on\n\n"
            "    -appport start,end : Specify port range for launched applications (default: %3,%4)\n"
            "    -homeapp           : Specify home application name\n\n"
            "The following are optional, but both must be specified to connect to an existing\n"
            "application instance (which must have QtUITest plugin loaded) :\n\n"
            "    -prestartapp       : Specify application name of existing application\n"
            "    -prestartport      : Specify port of existing application\n\n"
            ).arg(arguments[0]).arg(DEFAULT_PORT).arg(DEFAULT_APPPORTMIN).arg(DEFAULT_APPPORTMAX)
            .toLatin1().data();
            exit(0);
        } else if (QString("-debug").indexOf(arg) == 0) {
            m_debug = true;
        } else if (QString("-port").indexOf(arg) == 0) {
            if (i == args-1) {
                qWarning() << "Must provide port";
                exit(1);
            } else {
                QString portRange = arguments.at(++i);
                int commaPos = portRange.indexOf(QChar(','));
                if (commaPos == -1) {
                    m_port = portRange.toInt();
                    m_portMax = m_port;
                } else {
                    m_port = portRange.left(commaPos).toInt();
                    m_portMax = portRange.mid(commaPos+1).toInt();
                }
                if (!m_port || !m_portMax) {
                    qWarning() << "Invalid port specified";
                    exit(1);
                }
            }
        } else if (QString("-appport").indexOf(arg) == 0) {
            if (i == args-1) {
                qWarning() << "Must provide port range";
                exit(1);
            } else {
                QString appPortRange = arguments.at(++i);
                int commaPos = appPortRange.indexOf(QChar(','));
                if (commaPos == -1) {
                    qWarning() << "Must specify port range start,end";
                    exit(1);
                }
                m_appPortMin = appPortRange.left(commaPos).toInt();
                m_appPortMax = appPortRange.mid(commaPos+1).toInt();
                if (!m_appPortMin || !m_appPortMax) {
                    qWarning() << "Invalid port specified";
                    exit(1);
                }
            }
        } else if (QString("-prestartport").indexOf(arg) == 0) {
            if (i == args-1) {
                qWarning() << "Must provide port";
                exit(1);
            } else {
                m_prestartPort = arguments.at(++i).toInt();
                if (!m_prestartPort) {
                    qWarning() << "Invalid port specified";
                    exit(1);
                }
            }
        } else if (QString("-prestartapp").indexOf(arg) == 0) {
            if (i == args-1) {
                qWarning() << "Must provide prestarted application name";
                exit(1);
            } else {
                m_prestartApp = arguments.at(++i);
            }
        } else if (QString("-homeapp").indexOf(arg) == 0) {
            if (i == args-1) {
                qWarning() << "Must provide home application name";
                exit(1);
            } else {
                m_homeApp = arguments.at(++i);
            }
        } else if (QString("-config").indexOf(arg) == 0) {
            if (i == args-1) {
                qWarning() << "Must provide config value";
                exit(1);
            } else {
                QString conf = arguments.at(++i);
                if (!conf.contains(QLatin1Char('='))) {
                    qWarning() << "Config argument must be of format key=value";
                    exit(1);
                }
                deviceConfig << conf;
            }
        }
    }

    if ( (m_prestartPort && m_prestartApp.isNull()) ||
        (!m_prestartPort && !m_prestartApp.isNull()) ) {
        qWarning() << "Must specify both prestart port and application name";
        exit(1);
    }

    foreach (const QString& conf, deviceConfig) {
        int equals = conf.indexOf(QLatin1Char('='));
        QString key = conf.left(equals);
        QString value = conf.mid(equals+1);
        m_deviceConfig[key] = value;
    }
}

QtUiTestDSettings::~QtUiTestDSettings()
{
}

QtUiTestDSettings *QtUiTestDSettings::instance()
{
    static QtUiTestDSettings instance;
    return &instance;
}