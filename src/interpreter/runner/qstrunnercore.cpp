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

#include "qstrunnercore.h"
#include <private/qdeclarativecontext_p.h>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QtDebug>

QstRunnerCore::QstRunnerCore(QObject *parent) :
    QObject(parent)
{
}



QVariant QstRunnerCore::locationOf(QObject *o)
{
    QVariantMap ret;
    if (!o) {
        return ret;
    }
    QDeclarativeData *d = QDeclarativeData::get(o, false);
    if (d && d->outerContext && !d->outerContext->url.isEmpty()) {
        ret["url"] = d->outerContext->url;
        ret["lineNumber"] = d->lineNumber;
    }
    return ret;
}


void QstRunnerCore::setRootProperty(const QString &name, QObject *obj)
{
    m_engine->rootContext()->setContextProperty(name, obj);
}

bool QstRunnerCore::setup(QDeclarativeEngine *engine, const QStringList &args)
{
    m_engine = engine;
    qDebug()<< "setting __runner : " << this;
    m_engine->rootContext()->setContextProperty("__runner", this);

    QVariant ret;
    qDebug()<< "Invoking qml __setup";
    QMetaObject::invokeMethod(this, "__setup",
                Q_RETURN_ARG(QVariant, ret),
                Q_ARG(QVariant, args) );
    return ret.toBool();
}

void QstRunnerCore::setTest(QObject *test)
{
    setRootProperty("__testRoot", test);
}


int QstRunnerCore::exec()
{
    qDebug()<< "Validate test and exec";
    QVariant ret;
    QMetaObject::invokeMethod(this, "__exec",
                Q_RETURN_ARG(QVariant, ret) );
    return ret.toInt();
}
