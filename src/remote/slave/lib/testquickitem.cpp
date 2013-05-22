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


#include "testquickitem.h"

#include <QQuickItem>
#include <QQuickCanvas>
#include <QMetaObject>
#include <QImage>
#include <QMetaProperty>
#include <private/qqmlmetatype_p.h>
#include <designersupport.h>
#include <QDebug>
#include <QQmlProperty>

TestQuickItem::TestQuickItem(QQuickItem* _q)
    : TestQuickObject(_q)
{
}

QRect TestQuickItem::geometry() const
{
    return mapToGlobal(quickItem()->boundingRect().toRect());
}

QPoint TestQuickItem::mapToGlobal(const QPoint& pos) const
{
    return quickItem()->canvas()->mapToGlobal(quickItem()->mapToScene(pos).toPoint());
}

QPoint TestQuickItem::mapFromGlobal(const QPoint& pos) const
{
    return quickItem()->mapFromScene(quickItem()->canvas()->mapFromGlobal(pos)).toPoint();
}

QRect TestQuickItem::mapToGlobal(const QRect& local) const
{
    QRect result = quickItem()->mapRectToScene(local).toRect();
    result.moveTopLeft(quickItem()->canvas()->mapToGlobal(result.topLeft()));
    return result;
}

QRect TestQuickItem::mapFromGlobal(const QRect& global) const
{
    QRect result = global;
    result.moveTopLeft(quickItem()->canvas()->mapFromGlobal(result.topLeft()));
    return quickItem()->mapRectFromScene(result).toRect();
}

bool TestQuickItem::grabImage(QImage &img) const
{
    if (quickItem()->width() <= 0 || quickItem()->height() <= 0
        || !quickItem()->canvas())
        return false;

    QRect geo(geometry());
    geo.moveTopLeft(geo.topLeft() - quickItem()->canvas()->pos());

    QImage fb = quickItem()->canvas()->grabFrameBuffer();
    img = fb.copy(geo);
    return true;
}

bool TestQuickItem::isVisible() const
{
    return quickItem()->isVisible() && (quickItem()->opacity() > 0);
}

bool TestQuickItem::hasContents() const
{
    return quickItem()->flags() & QQuickItem::ItemHasContents;
}

QObject* TestQuickItem::parent() const
{
    QObject *ret = quickItem()->parentItem();
    if (!ret)
        ret = quickItem()->canvas();

    return ret;
}

bool TestQuickItem::hasFocus() const
{
    return quickItem()->hasFocus();
}

bool isValidAnchorName(const QString &name)
{
    static QStringList anchorNameList(QStringList() << QLatin1String("anchors.top")
                                                    << QLatin1String("anchors.left")
                                                    << QLatin1String("anchors.right")
                                                    << QLatin1String("anchors.bottom")
                                                    << QLatin1String("anchors.verticalCenter")
                                                    << QLatin1String("anchors.horizontalCenter")
                                                    << QLatin1String("anchors.fill")
                                                    << QLatin1String("anchors.centerIn")
                                                    << QLatin1String("anchors.baseline"));

    return anchorNameList.contains(name);
}

QVariant TestQuickItem::objectProperty(const QString& name) const
{
    if (name.startsWith(QLatin1String("anchors."))) {
        if (isValidAnchorName(name)) {
            QQmlContext *context = qmlContext(q);
            if (!context) {
               return QVariant();
            }

            QPair<QString, QObject*> pair = DesignerSupport::anchorLineTarget(quickItem(), name, context);
            if (pair.second) {
                if (pair.first.isEmpty()) {
                    return nameForObject(pair.second);
                } else {
                    return QString("%1.%2").arg(nameForObject(pair.second)).arg(pair.first);
                }
            } else {
                return QVariant();
            }
        } else {
            QObject *anchors = QQmlMetaType::toQObject(quickItem()->property("anchors"));
            if (anchors) {
                return anchors->property(name.mid(8).toLatin1());
            }
        }
    }

    QQmlProperty qmlProperty(q, name);
    QVariant ret = qmlProperty.read();

    if (!ret.isValid()) {
        ret = TestQuickObject::objectProperty(name);
    }

    return ret;
}

const QLatin1String TestQuickItem::containedInProperty("CONTAINED_IN");
const QLatin1String TestQuickItem::containsProperty("CONTAINS");
const QLatin1String TestQuickItem::intersectsProperty("INTERSECTS");
const QLatin1String TestQuickItem::labelProperty("LABEL");

bool TestQuickItem::matchProperty(const QString& propertyUpper, const QVariant &value) const
{
    static QStringList rectProperties(QStringList()
        << containedInProperty << containsProperty << intersectsProperty);

    if (rectProperties.contains(propertyUpper)) {
        QRect rect = value.toRect();
        if (rect.isValid()) {
            QRect geom = geometry();
            if ((propertyUpper == intersectsProperty && geom.intersects(rect)) ||
                (propertyUpper == containedInProperty && rect.contains(geom)) ||
                (propertyUpper == containsProperty && geom.contains(rect))) {
                return true;
            }
        } else {
            QtUiTest::setErrorString(QString("Invalid QRect specified for property '%1'").arg(propertyUpper));
            return false;
        }
    } else if (propertyUpper == labelProperty) {
        // Slightly nasty logic to support legacy LABEL functionality
        foreach (const QObject *child, children()) {
            if (child->inherits("QQuickText") && child->property("text") == value) {
                return true;
            } else if (child->inherits("QQuickRow")) {
                QtUiTest::Object *qo = qtuitest_cast<QtUiTest::Object*>(child);
                if (qo) {
                    foreach (const QObject *gChild, qo->children()) {
                        if (gChild->inherits("QQuickText") && gChild->property("text") == value) {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return TestQuickObject::matchProperty(propertyUpper, value);
}

QVariantMap TestQuickItem::getProperties() const
{
    QVariantMap map = TestQuickObject::getProperties();

    // Include attached anchors properties
    QObject *anchors = QQmlMetaType::toQObject(quickItem()->property("anchors"));
    if (anchors) {
        const QMetaObject *amo = anchors->metaObject();
        for (int i=0; i<amo->propertyCount(); i++) {
            QString anchorProperty = QString("anchors.%1").arg(amo->property(i).name());
            QVariant value = getProperty(anchorProperty);
            if (value.isValid()) {
                map[anchorProperty] = value;
            }
        }
    }

    return map;
}

bool TestQuickItem::ignoreScan() const
{
    return !(quickItem()->isEnabled());
}

QString TestQuickItem::nameForObject(QObject* object) const
{
    if (object == quickItem()->parentItem()) {
        return QLatin1String("parent");
    }
    return TestQuickObject::nameForObject(object);
}

QObject *TestQuickItem::window() const
{
    return quickItem()->canvas();
}

QString TestQuickItem::debugString() const
{
    QRect g(geometry());
    QString name(nameForObject(q));
    QString text = getProperty("text").toString();

    return QString("%1x:%2 y:%3 w:%4 h:%5%6%7").arg(name.isEmpty() ? QString() : QString("id:%1 ").arg(name)).arg(g.x()).arg(g.y()).arg(g.width()).arg(g.height()).arg(isVisible() ? QLatin1String(" visible") : QString()).arg(text.isEmpty() ? QString() : QString(" text('%1')").arg(text));
}
