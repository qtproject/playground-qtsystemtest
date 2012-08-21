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


#include <QtSystemTest/QstConnection>
#include "qtestbrowserdlg_p.h"
#include <QApplication>

#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT 5657

int main(int argc, char *argv[])
{
    QApplication *app = new QApplication(argc, argv);

    const QStringList arguments = QCoreApplication::arguments();
    int args = arguments.size();
    QStringList deviceConfig;

    QString hostName(DEFAULT_HOST);
    int port = DEFAULT_PORT;
    for (int i=1; i<args; ++i) {
        QString arg = arguments.at(i);
        if (QString("-help").indexOf(arg) == 0) {
            qWarning() << QString(
            "%1 - Object Browser\n\n"
            "    -help              : Display this help text and exit\n"
            "    -authost           : Host name of system under test (default: %2)\n"
            "    -port port         : Port qtuitestd is listening on (default: %3)\n"
            ).arg(arguments[0]).arg(DEFAULT_HOST).arg(DEFAULT_PORT).toLatin1().data();
            exit(0);
        } else if (QString("-authost").indexOf(arg) == 0) {
            if (i == args-1) {
                qWarning() << "Must provide host name";
                exit(1);
            } else {
                hostName = arguments.at(++i);
            }
        } else if (QString("-port").indexOf(arg) == 0) {
            if (i == args-1) {
                qWarning() << "Must provide port";
                exit(1);
            } else {
                port = arguments.at(++i).toInt();
                if (port == 0) {
                    qWarning() << "Invalid port";
                    exit(1);
                }
            }
        }
    }

    //TODO: Parse command line args for host and port
    QstConnection *connection = new QstConnection(hostName, port);
    if (!connection->connectToRemote()) {
        qWarning() << "Failed to connect to host" << hostName << "on port" << port;
        exit(1);
    }
    QTestBrowserDlg *browser = new QTestBrowserDlg(connection);
    browser->exec();

    delete browser;
    delete connection;
    delete app;
    return 0;
}
