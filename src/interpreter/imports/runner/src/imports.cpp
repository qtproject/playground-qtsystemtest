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
#include "qstxunitwriter.h"
#include <QtDebug>
#include <QtDeclarative>

void QtSystemTestRunnerModule::registerTypes(const char *uri)
{
    qDebug() << Q_FUNC_INFO << uri;
    Q_ASSERT(QLatin1String(uri) == QLatin1String("QtSystemTest.Runner"));
    qmlRegisterType<QstPlainTextWriter>(uri, 1, 0, "PlainTextWriter");
    qmlRegisterType<QstXmlStreamWriter>(uri, 1, 0, "XmlWriter");
    qmlRegisterType<QstXUnitWriter>(uri, 1, 0, "XUnitWriter");
}

void QtSystemTestRunnerModule::initializeEngine(QDeclarativeEngine *, const char *)
{
}

Q_EXPORT_PLUGIN2(qtsystemtestrunner, QtSystemTestRunnerModule);
