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


#include "testwidget.h"

#include <QDebug>
#include <QWidget>
#include <QPixmap>
#include <QAction>
#include <QMetaProperty>

const QLatin1String TestWidget::actionsProperty("QACTIONS");
const QLatin1String TestWidget::classNameProperty("CLASSNAME");
const QLatin1String TestWidget::hasContentsProperty("HAS_CONTENTS");
const QLatin1String TestWidget::inheritsProperty("INHERITS");
const QLatin1String TestWidget::propertyProperty("PROPERTY");

TestWidget::TestWidget(QWidget* _q)
    : TestObject(_q)
{}

TestWidget::~TestWidget()
{
}

QRect TestWidget::geometry() const
{
    QWidget *w = widget();
    QRect geo(w->geometry());
    geo.moveTopLeft(w->mapToGlobal(w->mapFromParent(geo.topLeft())));
    return geo;
}

bool TestWidget::isVisible() const
{
    return widget()->isVisible();
}

bool TestWidget::hasContents() const
{
    return true;
}

QObject* TestWidget::parent() const
{
    return q->parent();
}

QPoint TestWidget::mapToGlobal(const QPoint &local) const
{
    return widget()->mapToGlobal(local);
}

QPoint TestWidget::mapFromGlobal(const QPoint &global) const
{
    return widget()->mapFromGlobal(global);
}

QRect TestWidget::mapToGlobal(const QRect &local) const
{
    QRect result = local;
    result.moveTopLeft(mapToGlobal(result.topLeft()));
    return result;
}

QRect TestWidget::mapFromGlobal(const QRect &global) const
{
    QRect result = global;
    result.moveTopLeft(mapFromGlobal(result.topLeft()));
    return result;
}

bool TestWidget::grabImage(QImage &img) const
{
    img = QPixmap::grabWidget(widget()).toImage();
    return !img.isNull();
}

const QObjectList TestWidget::children() const
{
    QObjectList ret;
    foreach (QObject *child, q->children()) {
        if (child->isWidgetType()) {
            ret << child;
        }
    }

    return ret;
}

bool TestWidget::hasFocus() const
{
    return widget()->hasFocus();
}

QVariantList TestWidget::getActions() const
{
    QVariantList ret;
    foreach (QAction *action, widget()->actions()) {
        ret << QVariant::fromValue(static_cast<QObject*>(action));
    }
    return ret;
}

QVariant TestWidget::getProperty(const QString& name) const
{
    if (name.toUpper() == actionsProperty) {
        return getActions();
    } else if (name.toUpper() == QLatin1String("INHERITANCE")) {
        return inheritance();
    }
    return q->property(name.toLocal8Bit());
}

bool TestWidget::matchProperty(const QString& propertyUpper, const QVariant &value) const
{
    return (propertyUpper == propertyProperty && getProperty(value.toString()).isValid())
        || (propertyUpper == classNameProperty && q->metaObject()->className() == value)
        || (propertyUpper == inheritsProperty && q->inherits(value.toString().toLatin1()))
        || (propertyUpper == hasContentsProperty);
}


QVariantList TestWidget::resources() const
{
    return getActions();
}

QObject* TestWidget::window() const
{
    return widget()->window();
}

QString TestWidget::debugString() const
{
    QRect g(geometry());
    QString name;

    return QString("%1x:%2 y:%3 w:%4 h:%5%6").arg(name.isEmpty() ? QString() : QString("id:%1 ").arg(name)).arg(g.x()).arg(g.y()).arg(g.width()).arg(g.height()).arg(isVisible() ? QLatin1String(" visible") : QString());
}
