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


#include "testapplication.h"
#include "testquickfactory.h"

#include "testwidget.h"
#include <QWidget>
#include <QApplication>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    #include "testquickitem.h"
    #include "testquickobject.h"
    #include "testquickview.h"
    #include "testwindow.h"
    #include <QQuickView>
    #include <QQuickItem>
    #include <QWindow>
#endif

TestQuickFactory::TestQuickFactory()
{
}

QObject* TestQuickFactory::create(QObject* o)
{
    if (o->isWidgetType()) return new TestWidget(qobject_cast<QWidget*>(o));

    QApplication *app = qobject_cast<QApplication*>(o);
    if (app) return new TestApplication(app);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QQuickView *view = qobject_cast<QQuickView*>(o);
    if (view) return new TestQuickView(view);

    QQuickItem *item = qobject_cast<QQuickItem*>(o);
    if (item) return new TestQuickItem(item);

    QWindow *window = qobject_cast<QWindow*>(o);
    if (window) return new TestWindow(window);

    return new TestQuickObject(o);
#else
    // FIXME: Need support for Quick 1!
    return 0;
#endif
}

QStringList TestQuickFactory::keys() const
{
    return QStringList()
        << "QQuickItem"
        << "QQuickView"
        << "QWindow"
        << "QGuiApplication"
        << "QObject"
        ;
}
