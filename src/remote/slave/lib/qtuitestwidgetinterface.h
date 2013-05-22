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


#ifndef QTUITESTWIDGETINTERFACE_H
#define QTUITESTWIDGETINTERFACE_H

#include <QStringList>
#include <QVariantList>
#include <QSet>
#include <QtGlobal>

#include <QtSystemTest/qstglobal.h>

#include "qtuitestnamespace.h"

class QRect;
class QRegion;
class QPoint;
class QWindow;

namespace QtUiTest
{
    template<class T> T qtuitest_cast_helper(QObject*,T);

#define QTUITEST_INTERFACE(Klass)                                    \
    public:                                                            \
        virtual ~Klass() {}                                            \
        static const char* _q_interfaceName() { return #Klass; }       \
    private:                                                           \
        template<class T> friend T qtuitest_cast_helper(QObject*,T);

    class QTUITEST_EXPORT WidgetFactory
    {
        QTUITEST_INTERFACE(WidgetFactory)
    public:
        virtual QObject* create(QObject*) = 0;
        virtual QStringList keys() const = 0;
    };

    class QTUITEST_EXPORT Object
    {
        QTUITEST_INTERFACE(Object)
    public:
        virtual QObject* parent() const = 0;
        virtual const QObjectList children() const = 0;
        QObjectList ancestors() const;
        virtual void descendants(QSet<QObject*> &descendants) const;
        QObjectList descendants() const;
        virtual QVariant getProperty(const QString&) const;
        virtual bool setProperty(const QString&, const QVariant&);
        virtual bool matchProperty(const QString&, const QVariant&) const;
        virtual QVariantMap getProperties() const;
        virtual QString typeOf() const;
        virtual QStringList inheritance() const;
        virtual QVariantList resources() const;
        virtual bool hasContents() const;
        virtual QString debugString() const;
    };

    class QTUITEST_EXPORT Widget
    {
        QTUITEST_INTERFACE(Widget)
    public:
        virtual QRect geometry() const = 0;
        virtual int x() const;
        virtual int y() const;
        virtual int width() const;
        virtual int height() const;
        virtual bool isVisible() const = 0;
        virtual QPoint mapToGlobal(const QPoint&) const = 0;
        virtual QPoint mapFromGlobal(const QPoint&) const = 0;
        virtual QRect mapToGlobal(const QRect&) const = 0;
        virtual QRect mapFromGlobal(const QRect&) const = 0;
        virtual QPoint center() const;
        virtual bool hasFocus() const = 0;
        virtual QObject* focusProxy() const;
        virtual bool grabImage(QImage &img) const;
        virtual bool ignoreScan() const;
        virtual QObject* window() const;
    };

};

Q_DECLARE_INTERFACE(
        QtUiTest::WidgetFactory,
        "com.nokia.qt.QtUiTest.WidgetFactory/1.0")
Q_DECLARE_INTERFACE(
        QtUiTest::Object,
        "com.nokia.qt.QtUiTest.Object/1.0")
Q_DECLARE_INTERFACE(
        QtUiTest::Widget,
        "com.nokia.qt.QtUiTest.Widget/1.0")
#endif

