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


#include "qeventwatcher_p.h"

#include <QSet>
#include <QPointer>
#include <QCoreApplication>
#include <QtSystemTest/QstDebug>

struct QEventWatcherPrivate
{
    QSet<QObject*>              objects;
    QList<QEventWatcherFilter*> filters;
    int                         count;
};

QEventWatcher::QEventWatcher(QObject* parent)
    : QObject(parent),
      d(new QEventWatcherPrivate)
{
    QTT_TRACE_FUNCTION();
    d->count = 0;
}

QEventWatcher::~QEventWatcher()
{
    QTT_TRACE_FUNCTION();
    qDeleteAll(d->filters);
    delete d;
    d = 0;
}

void QEventWatcher::addObject(QObject* obj)
{
    QTT_TRACE_FUNCTION();
    d->objects << obj;
    qApp->installEventFilter(this);
}

void QEventWatcher::addFilter(QEventWatcherFilter* filter)
{
    QTT_TRACE_FUNCTION();
    d->filters << filter;
}

int QEventWatcher::count() const
{
    QTT_TRACE_FUNCTION();
    return d->count;
}

QString QEventWatcher::toString() const
{
    QTT_TRACE_FUNCTION();
    QString ret;
    QString sep;
    for (int i = d->filters.count()-1; i >= 0; --i) {
        ret += sep + d->filters.at(i)->toString();
        sep = ", ";
    }
    return ret;
}

bool QEventWatcher::eventFilter(QObject* obj, QEvent* e)
{
    QTT_TRACE_FUNCTION();
    if (!d->objects.contains(obj)) return false;

    bool accept = (d->filters.count() ? false : true);
    for (int i = d->filters.count()-1; i >= 0 && !accept; --i) {
        accept = d->filters.at(i)->accept(obj,e);
    }

    if (accept) {
        ++d->count;
        emit event(obj, e->type());
    }

    return false;
}

