/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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


#include "testquickview.h"

#include <QDebug>
#include <QQuickView>
#include <QQuickItem>
#include <QImage>

TestQuickView::TestQuickView(QQuickView* _q)
    : TestQuickObject(_q)
{}

TestQuickView::~TestQuickView()
{
}

QRect TestQuickView::geometry() const
{
    return static_cast<QQuickView*>(q)->geometry();
}

bool TestQuickView::isVisible() const
{
    return true;
}

bool TestQuickView::hasContents() const
{
    return true;
}

QObject* TestQuickView::parent() const
{
    return static_cast<QQuickView*>(q)->parent();
}

QPoint TestQuickView::mapToGlobal(const QPoint &local) const
{
    return static_cast<QQuickView*>(q)->mapToGlobal(local);
}

QPoint TestQuickView::mapFromGlobal(const QPoint &global) const
{
    return static_cast<QQuickView*>(q)->mapFromGlobal(global);
}

QRect TestQuickView::mapToGlobal(const QRect &local) const
{
    QRect result = local;
    result.moveTopLeft(mapToGlobal(result.topLeft()));
    return result;
}

QRect TestQuickView::mapFromGlobal(const QRect &global) const
{
    QRect result = global;
    result.moveTopLeft(mapFromGlobal(result.topLeft()));
    return result;
}

bool TestQuickView::grabImage(QImage &img) const
{
    img = static_cast<QQuickView*>(q)->grabFrameBuffer();
    return !img.isNull();
}

const QObjectList TestQuickView::children() const
{
    QObjectList ret;

    QObject *rootObject = static_cast<QQuickView*>(q)->rootObject();
    if (rootObject)
        ret << rootObject;

    return ret;
}

bool TestQuickView::hasFocus() const
{
    return true;
}

QString TestQuickView::debugString() const
{
    QRect g(geometry());
    QString name(nameForObject(q));

    return QString("%1x:%2 y:%3 w:%4 h:%5%6").arg(name.isEmpty() ? QString() : QString("id:%1 ").arg(name)).arg(g.x()).arg(g.y()).arg(g.width()).arg(g.height()).arg(isVisible() ? QLatin1String(" visible") : QString());
}
