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


#ifndef QTUITESTWIDGETS_P_H
#define QTUITESTWIDGETS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtUiTest API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QMetaType>
#include <QHash>
#include <Qt>

#include "qtuitestnamespace.h"

class QtUiTestWidgetsPrivate;
class QPoint;

class QTUITEST_EXPORT QtUiTestWidgets : public QObject
{
    Q_OBJECT

public:
    static QtUiTestWidgets* instance();

    virtual ~QtUiTestWidgets();

    void setInputOption(QtUiTest::InputOption,bool = true);
    bool testInputOption(QtUiTest::InputOption) const;

    QString errorString() const;
    void setErrorString(const QString&);
    QDebug debug() const;
    QString debugOutput();

    void registerFactory(QtUiTest::WidgetFactory*);

    void touchPress(const QVariantList &);
    void touchUpdate(const QVariantList &);
    void touchRelease(const QVariantList &);

    void mousePress  (const QPoint&,Qt::MouseButtons = Qt::LeftButton,
            QtUiTest::InputOption = QtUiTest::NoOptions);
    void mouseRelease(const QPoint&,Qt::MouseButtons = Qt::LeftButton,
            QtUiTest::InputOption = QtUiTest::NoOptions);
    void mouseClick  (const QPoint&,Qt::MouseButtons = Qt::LeftButton,
            QtUiTest::InputOption = QtUiTest::NoOptions);
    void mouseDClick (const QPoint&,Qt::MouseButtons = Qt::LeftButton,
            QtUiTest::InputOption = QtUiTest::NoOptions);
    void mouseMove   (const QPoint&, QtUiTest::InputOption = QtUiTest::NoOptions);

    void keyPress  (Qt::Key,Qt::KeyboardModifiers = 0,QtUiTest::InputOption = QtUiTest::NoOptions);
    void keyRelease(Qt::Key,Qt::KeyboardModifiers = 0,QtUiTest::InputOption = QtUiTest::NoOptions);
    void keyClick  (Qt::Key,Qt::KeyboardModifiers = 0,QtUiTest::InputOption = QtUiTest::NoOptions);

private:
    Q_DISABLE_COPY(QtUiTestWidgets)

    friend class QtUiTestWidgetsPrivate;

    QtUiTestWidgetsPrivate* d;

    QtUiTestWidgets();
    QObject* testWidget(QObject*,const QByteArray&);

    void refreshPlugins();
    void clear();

    Q_PRIVATE_SLOT(d, void _q_objectDestroyed())
    Q_PRIVATE_SLOT(d, void _q_postNextKeyEvent())
    Q_PRIVATE_SLOT(d, void _q_postNextMouseEvent())
    Q_PRIVATE_SLOT(d, void _q_postNextTouchEvent())

    friend QObject* QtUiTest::testWidget(QObject*,const char*);
};

#endif

