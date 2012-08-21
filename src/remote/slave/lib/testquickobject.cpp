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


#include "testquickobject.h"

#include <QQuickItem>
#include <QQmlContext>
#include <QQmlListProperty>
#include <QMetaObject>
#include <QMetaProperty>
#include <private/qqmlmetatype_p.h>

const QLatin1String TestQuickObject::classNameProperty("CLASSNAME");
const QLatin1String TestQuickObject::hasContentsProperty("HAS_CONTENTS");
const QLatin1String TestQuickObject::inheritsProperty("INHERITS");
const QLatin1String TestQuickObject::propertyProperty("PROPERTY");

const QLatin1String TestQuickObject::qmlTypeProperty("QMLTYPE");
const QLatin1String TestQuickObject::qmlIdProperty("QMLID");
const QLatin1String TestQuickObject::qmlInheritsProperty("QMLINHERITS");

TestQuickObject::TestQuickObject(QObject* _q)
    : q(_q)
{
}

const QObjectList TestQuickObject::children() const
{
    QObjectList ret;
    QQuickItem *item = qobject_cast<QQuickItem*>(q);
    if (item) {
        foreach (QQuickItem *child, item->childItems()) {
            ret << child;
        }
    }

    return ret;
}

QObject* TestQuickObject::parent() const
{
    return 0;
}

QVariant TestQuickObject::objectProperty(const QString& name) const
{
    return q->property(name.toLocal8Bit());
}

QVariant TestQuickObject::getProperty(const QString& name) const
{
    // Do QtUiTest property type check
    if (name.toUpper() == qmlTypeProperty) {
        return typeOf();
    } else if (name.toUpper() == qmlIdProperty) {
        return nameForObject(q);
    }

    // Get the property and convert it to QtUiTest compatible format
    QVariant ret = objectProperty(name);

    if (!ret.isValid()) {
        return ret;
    }

    if (ret.canConvert<QQmlListReference>()) {
        QQmlListReference ref = ret.value<QQmlListReference>();
        QVariantList list;
        for (int i=0; i<ref.count(); ++i) {
            list << QVariant::fromValue(ref.at(i));
        }
        ret = list;
    } else {
        const QMetaObject *mo = q->metaObject();
        QMetaProperty mp = mo->property(mo->indexOfProperty(name.toLocal8Bit()));
        if (mp.isEnumType())
        {
            QVariantMap map;
            int value = ret.toInt();
            map["_enum_"] = mp.enumerator().valueToKeys(value);
            map["_value_"] = value;
            ret = map;
        }
    }

    if (ret.type() == QVariant::UserType) {
        bool ok;
        QObject *obj = QQmlMetaType::toQObject(ret, &ok);
        if (ok) {
            return QVariant::fromValue(obj);
        }
    }

    return ret;
}

bool TestQuickObject::setProperty(const QString& name, const QVariant& value)
{
    return q->setProperty(name.toAscii(), value);
}

bool TestQuickObject::matchProperty(const QString& propertyUpper, const QVariant &value) const
{
    return (propertyUpper == propertyProperty && getProperty(value.toString()).isValid())
        || (propertyUpper == classNameProperty && q->metaObject()->className() == value)
        || (propertyUpper == inheritsProperty && q->inherits(value.toString().toLatin1()))
        || (propertyUpper == hasContentsProperty && hasContents() == value.toBool())
        || (propertyUpper == qmlInheritsProperty && inheritsQmlType(value.toString()));
}

QVariantMap TestQuickObject::getProperties() const
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

QVariantList TestQuickObject::resources() const
{
    return getProperty(QLatin1String("resources")).toList();
}

QString TestQuickObject::typeOf() const
{
    return getTypeName();
}

bool TestQuickObject::inheritsQmlType(const QString &typeName) const
{
    const QMetaObject *mo = q->metaObject();

    while (mo) {
        if (typeNameForMetaObject(mo) == typeName) {
            return true;
        }
        if (mo == &QQuickItem::staticMetaObject) {
            break;
        }
        mo = mo->superClass();
    }

    return false;
}

QString TestQuickObject::getTypeName() const
{
    return typeNameForMetaObject(q->metaObject());
}

QString TestQuickObject::typeNameForMetaObject(const QMetaObject *mo) const
{
    QString typeName;
    if (!mo) return typeName;

    QQmlType *type = QQmlMetaType::qmlType(mo);

    // This logic is copied from QQmlInfo
    if (type) {
        typeName = type->qmlTypeName();
        int lastSlash = typeName.lastIndexOf(QLatin1Char('/'));
        if (lastSlash != -1)
            typeName = typeName.mid(lastSlash+1);
    } else {
        typeName = QString::fromUtf8(mo->className());
        int marker = typeName.indexOf(QLatin1String("_QMLTYPE_"));
        if (marker != -1)
            typeName = typeName.left(marker);

        marker = typeName.indexOf(QLatin1String("_QML_"));
        if (marker != -1) {
            typeName = typeName.left(marker);
            typeName += QLatin1Char('*');
            type = QQmlMetaType::qmlType(QMetaType::type(typeName.toLatin1()));
            if (type) {
                typeName = type->qmlTypeName();
                int lastSlash = typeName.lastIndexOf(QLatin1Char('/'));
                if (lastSlash != -1)
                    typeName = typeName.mid(lastSlash+1);
            }
        }
    }

    if (typeName.isNull()) {
        return mo->className();
    }

    return typeName;
}

QString TestQuickObject::nameForObject(QObject* object) const
{
    QQmlContext *context = qmlContext(q);
    if (!object || !context) {
        return QString();
    }
    return context->nameForObject(object);
}

QStringList TestQuickObject::inheritance() const
{
    QStringList ret;

    const QMetaObject *mo = q->metaObject();

    while (mo) {
        ret << typeNameForMetaObject(mo);
        if (mo == &QQuickItem::staticMetaObject) {
            break;
        }
        mo = mo->superClass();
    }

    return ret;
}