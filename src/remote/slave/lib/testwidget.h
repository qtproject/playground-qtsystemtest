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


#ifndef TEST_WIDGET_H
#define TEST_WIDGET_H

#include "testobject.h"
#include "qtuitestwidgetinterface.h"

#include <QWidget>

class TestWidget : public TestObject,
    public QtUiTest::Widget
{
    Q_OBJECT
    Q_INTERFACES(QtUiTest::Object)
    Q_INTERFACES(QtUiTest::Widget)

public:
    TestWidget(QWidget*);
    ~TestWidget();

    virtual bool matchProperty(const QString&, const QVariant&) const;
    virtual QRect geometry() const;
    virtual bool isVisible() const;
    virtual bool hasContents() const;
    virtual QObject* parent() const;
    virtual QPoint mapToGlobal(const QPoint&) const;
    virtual QPoint mapFromGlobal(const QPoint&) const;
    virtual QRect mapToGlobal(const QRect&) const;
    virtual QRect mapFromGlobal(const QRect&) const;
    virtual bool grabImage(QImage &img) const;
    virtual bool hasFocus() const;
    virtual const QObjectList children() const;
    virtual QVariant getProperty(const QString&) const;
    virtual QVariantList resources() const;
    virtual QObject* window() const;
    virtual QString debugString() const;

protected:
    inline QWidget *widget() const { return static_cast<QWidget*>(q); }
    QVariantList getActions() const;
    static const QLatin1String actionsProperty;
    static const QLatin1String classNameProperty;
    static const QLatin1String hasContentsProperty;
    static const QLatin1String inheritsProperty;
    static const QLatin1String propertyProperty;

};

#endif

