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


#ifndef TEST_QUICK_OBJECT_H
#define TEST_QUICK_OBJECT_H

#include "qtuitestwidgetinterface.h"

class QQmlContext;

class TestQuickObject : public QObject,
    public QtUiTest::Object
{
    Q_OBJECT
    Q_INTERFACES(QtUiTest::Object)

public:

    TestQuickObject(QObject*);

    virtual const QObjectList children() const;
    virtual QObject* parent() const;
    virtual QVariant getProperty(const QString&) const;
    virtual bool setProperty(const QString&, const QVariant&);
    virtual bool matchProperty(const QString&, const QVariant&) const;
    virtual QVariantMap getProperties() const;
    virtual QVariantList resources() const;
    virtual QString typeOf() const;
    virtual QStringList inheritance() const;

protected:
    bool inheritsQmlType(const QString&) const;
    virtual QString getTypeName() const;
    QString typeNameForMetaObject(const QMetaObject *mo) const;
    virtual QString nameForObject(QObject*) const;
    virtual QVariant objectProperty(const QString& name) const;

    static const QLatin1String qmlTypeProperty;
    static const QLatin1String qmlIdProperty;
    static const QLatin1String qmlInheritsProperty;

    static const QLatin1String classNameProperty;
    static const QLatin1String hasContentsProperty;
    static const QLatin1String inheritsProperty;
    static const QLatin1String propertyProperty;

    QObject* q;
};

#endif
