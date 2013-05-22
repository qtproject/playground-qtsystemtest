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


#include "testwindow.h"

#include <QDebug>
#include <QWindow>
#include <QScreen>

TestWindow::TestWindow(QWindow* _q)
    : TestQuickObject(_q)
{}

TestWindow::~TestWindow()
{
}

QRect TestWindow::geometry() const
{
    return window()->geometry();
}

bool TestWindow::isVisible() const
{
    return window()->isExposed();
}

bool TestWindow::hasContents() const
{
    return true;
}

QObject* TestWindow::parent() const
{
    return window()->parent();
}

QPoint TestWindow::mapToGlobal(const QPoint &local) const
{
    return window()->mapToGlobal(local);
}

QPoint TestWindow::mapFromGlobal(const QPoint &global) const
{
    return window()->mapFromGlobal(global);
}

QRect TestWindow::mapToGlobal(const QRect &local) const
{
    QRect result = local;
    result.moveTopLeft(mapToGlobal(result.topLeft()));
    return result;
}

QRect TestWindow::mapFromGlobal(const QRect &global) const
{
    QRect result = global;
    result.moveTopLeft(mapFromGlobal(result.topLeft()));
    return result;
}

bool TestWindow::grabImage(QImage &img) const
{
    Q_UNUSED(img);
    return false;
}

const QObjectList TestWindow::children() const
{
    QObjectList ret;

    foreach (QObject *child, q->children()) {
        if (child->isWindowType() || child->isWidgetType()) {
            ret << child;
        }
    }

    // This is all a bit nasty...
    QObject *rootObject = window()->focusObject();
    while (rootObject) {
        QObject *parent = rootObject->parent();
        if (!parent || parent == q || !parent->isWidgetType())
            break;
        rootObject = parent;
    }
    if (rootObject)
        ret << rootObject;

    return ret;
}

bool TestWindow::hasFocus() const
{
    return true;
}

QString TestWindow::debugString() const
{
    QRect g(geometry());
    QString name(nameForObject(q));

    return QString("%1x:%2 y:%3 w:%4 h:%5%6").arg(name.isEmpty() ? QString() : QString("id:%1 ").arg(name)).arg(g.x()).arg(g.y()).arg(g.width()).arg(g.height()).arg(isVisible() ? QLatin1String(" visible") : QString());
}
