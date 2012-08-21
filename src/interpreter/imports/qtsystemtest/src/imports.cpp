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

#include "imports.h"
#include "qstcoreapi.h"
#include "qstutils.h"
#include "qstlocalprocess.h"
#include <QtDeclarative>

#include <QtSystemTest/QstConnection>
#include <QtSystemTest/QstConnectionManager>

void QtSystemTestQmlModule::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("QtSystemTest"));
    qmlRegisterType<QstCoreApi>( uri, 1, 0, "CoreApi");
    qmlRegisterType<QstConnection>( uri, 1, 0, "Connection");
    qmlRegisterType<QstLocalProcess>( uri, 1, 0, "LocalProcess");
}

void QtSystemTestQmlModule::initializeEngine(QDeclarativeEngine *e, const char *uri)
{
    qDebug() << Q_FUNC_INFO << "Registering connection manager and Utils" << uri;
    e->rootContext()->setContextProperty("ConnectionManager", new QstConnectionManager());
    e->rootContext()->setContextProperty("Utils", new QstUtils());
}

Q_EXPORT_PLUGIN2(qtsystemtestplugin, QtSystemTestQmlModule);
