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


#include "testobject.h"

#include <QMetaObject>
#include <QMetaProperty>

const QLatin1String TestObject::classNameProperty("CLASSNAME");
const QLatin1String TestObject::hasContentsProperty("HAS_CONTENTS");
const QLatin1String TestObject::inheritsProperty("INHERITS");
const QLatin1String TestObject::propertyProperty("PROPERTY");

TestObject::TestObject(QObject* _q)
    : q(_q)
{
}

QObject* TestObject::parent() const
{
    return 0;
}

QVariant TestObject::getProperty(const QString& name) const
{
    return q->property(name.toLocal8Bit());
}

bool TestObject::setProperty(const QString& name, const QVariant& value)
{
    return q->setProperty(name.toAscii(), value);
}

bool TestObject::matchProperty(const QString& propertyUpper, const QVariant &value) const
{
    return (propertyUpper == propertyProperty && getProperty(value.toString()).isValid())
        || (propertyUpper == classNameProperty && q->metaObject()->className() == value)
        || (propertyUpper == inheritsProperty && q->inherits(value.toString().toLatin1()));
}

QVariantMap TestObject::getProperties() const
{
    QVariantMap map;
    const QMetaObject *mo = q->metaObject();
    for (int i=0; i<mo->propertyCount(); i++) {
        map[mo->property(i).name()] = getProperty(mo->property(i).name());
    }
    foreach (const QByteArray &prop, q->dynamicPropertyNames()) {
        map[prop] = getProperty(prop);
    }
    return map;
}

QString TestObject::typeOf() const
{
    return q->metaObject()->className();
}

QStringList TestObject::inheritance() const
{
    QStringList ret;

    const QMetaObject *mo = q->metaObject();

    while (mo) {
        ret << mo->className();
        mo = mo->superClass();
    }

    return ret;
}