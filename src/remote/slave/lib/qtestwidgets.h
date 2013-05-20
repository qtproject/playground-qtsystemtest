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


#ifndef QTESTWIDGETS_H
#define QTESTWIDGETS_H

#include <QHash>
#include <QMetaType>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include <QtSystemTest/qstglobal.h>

#include "qtuitestnamespace.h"

class QTestWidget;

class QTSLAVE_EXPORT QActiveTestWidgetData : public QObject
{
public:
    QActiveTestWidgetData();
    ~QActiveTestWidgetData();

    void clear();
    void removeTestWidget( QObject* );
    bool scan( QObject* aw, QtUiTest::QueryFlags flags=0 );

    QHash<QString, QObject*> testWidgetsBySignature;
    QHash<const QObject*, QString> signaturesByObject;
    QSet<QObject*> m_scannedItems;
};

class QTSLAVE_EXPORT QActiveTestWidget : public QObject
{
    Q_OBJECT

public:
    static QActiveTestWidget* instance();
    void clear();
    bool rescan( QtUiTest::QueryFlags flags=0 );
    QStringList descendantTree(QObject *obj);
    bool findWidget( const QString &signature, QObject *&widget, QString &error );
    QObjectList findObjectsByProperty( const QString&, const QVariant&, const QObjectList& = QObjectList() );
    QObjectList queryProperties( const QVariantMap&, QtUiTest::QueryFlags flags );
    QVariantMap getObjects(const QString&);
    QObject* testWidget( const QString &signature );
    QString signature( const QObject *) const;
    void registerTestWidget( QObject *ao );
    void setAutoScan(bool);
    bool forceRescan();

    static const QLatin1String NoActiveWidgetError;

    static const QLatin1String ancestorOfProperty;
    static const QLatin1String childOfProperty;
    static const QLatin1String descendantOfProperty;
    static const QLatin1String parentOfProperty;
    static const QLatin1String resourceOfProperty;

protected slots:
    void onDestroyed(QObject*);

private:
    QActiveTestWidget();
    ~QActiveTestWidget();

    bool auto_scan;
    QActiveTestWidgetData *d;
};

//
// This is the class that manages the test widgets and handles queries about them.
//
class QTSLAVE_EXPORT QTestWidgets
{
public:
    static QObject* activeWidget();
    static QVariant windowProperty(const QString&);
    static QObject* testWidget( const QString &signature );
    static QString signature( const QObject* object );
    static QStringList objectListToSignatures(const QObjectList&);

    static bool lessThan(QObject *a, QObject *b) {
        QtUiTest::Widget *wa = qtuitest_cast<QtUiTest::Widget*>(a);
        QtUiTest::Widget *wb = qtuitest_cast<QtUiTest::Widget*>(b);
        if (!wa) return true;
        if (!wb) return false;
        if (wa->y() == wb->y())
            return wa->x() < wb->x();
        else
            return wa->y() < wb->y();
    }
};

class PropertyMatcher
{
public:
    PropertyMatcher(const QVariant &searchValue, bool &ok);
    bool match(const QVariant &value) const;

private:
    bool equals(const QVariant &value) const;
    bool notEquals(const QVariant &value) const;
    bool containedIn(const QVariant &value) const;
    bool greaterThan(const QVariant &value) const;
    bool lessThan(const QVariant &value) const;
    bool regExpMatch(const QVariant &value) const;

    QVariant m_searchValue;
    QStringList m_searchList;
    QRegExp m_searchRegExp;
    bool (PropertyMatcher::*m_matchFunction)(const QVariant&) const;
    double m_searchDouble;
};

#endif
