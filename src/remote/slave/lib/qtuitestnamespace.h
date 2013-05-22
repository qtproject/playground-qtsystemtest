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


#ifndef QTUITESTNAMESPACE_H
#define QTUITESTNAMESPACE_H

#include <QtSystemTest/qstglobal.h>

#include <QEvent>
#include <QObject>
#include <Qt>
#include <QVariant>

class QPoint;

namespace QtUiTest
{
    enum InputOption {
        NoOptions      = 0x0,
        KeyRepeat      = 0x1
    };

    enum Key {
#ifdef Q_WS_QWS
        Key_Activate = Qt::Key_Select,
#else
        Key_Activate = Qt::Key_Enter,
#endif

#ifdef Q_WS_QWS
        Key_Select   = Qt::Key_Select,
#else
        Key_Select   = Qt::Key_Space,
#endif

#if defined(Q_WS_QWS) || defined (Q_OS_SYMBIAN)
        Key_ActivateButton   = Qt::Key_Select
#else
        Key_ActivateButton   = Qt::Key_Space
#endif
    };

    enum QueryFlag
    {
        NoSort            = 0x01,
        NoScan            = 0x02,
        ScanInvisible     = 0x04,
        ScanDisabledItems = 0x08,
        IgnoreScanned     = 0x10
    };
    Q_DECLARE_FLAGS(QueryFlags, QueryFlag);

    QTUITEST_EXPORT void setInputOption(InputOption,bool = true);
    QTUITEST_EXPORT bool testInputOption(InputOption);

    QTUITEST_EXPORT QString errorString();
    QTUITEST_EXPORT void setErrorString(const QString&);
    QTUITEST_EXPORT QDebug debug();
    QTUITEST_EXPORT QString debugOutput();

    QTUITEST_EXPORT void touchPress(const QVariantList&);
    QTUITEST_EXPORT void touchUpdate(const QVariantList&);
    QTUITEST_EXPORT void touchRelease(const QVariantList&);

    QTUITEST_EXPORT void mousePress  (const QPoint&,Qt::MouseButtons = Qt::LeftButton,
            InputOption = NoOptions);
    QTUITEST_EXPORT void mouseRelease(const QPoint&,Qt::MouseButtons = Qt::LeftButton,
            InputOption = NoOptions);
    QTUITEST_EXPORT void mouseClick  (const QPoint&,Qt::MouseButtons = Qt::LeftButton,
            InputOption = NoOptions);
    QTUITEST_EXPORT bool mouseClick  (QObject*,const QPoint&,Qt::MouseButtons = Qt::LeftButton,
            InputOption = NoOptions);
    QTUITEST_EXPORT bool mouseClick  (QObject*,const QByteArray&,const QPoint&,
            Qt::MouseButtons = Qt::LeftButton, InputOption = NoOptions);
    QTUITEST_EXPORT void mouseDClick  (const QPoint&,Qt::MouseButtons = Qt::LeftButton,
            InputOption = NoOptions);
    QTUITEST_EXPORT bool mouseDClick  (QObject*,const QPoint&,Qt::MouseButtons = Qt::LeftButton,
            InputOption = NoOptions);
    QTUITEST_EXPORT bool mouseDClick  (QObject*,const QByteArray&,const QPoint&,
            Qt::MouseButtons = Qt::LeftButton, InputOption = NoOptions);
    QTUITEST_EXPORT void mouseMove   (const QPoint&, InputOption = NoOptions);

    QTUITEST_EXPORT void keyPress  (int,Qt::KeyboardModifiers = 0,
            InputOption = NoOptions);
    QTUITEST_EXPORT void keyRelease(int,Qt::KeyboardModifiers = 0,
            InputOption = NoOptions);
    QTUITEST_EXPORT void keyClick  (int,Qt::KeyboardModifiers = 0,
            InputOption = NoOptions);
    QTUITEST_EXPORT bool keyClick  (QObject*,int,Qt::KeyboardModifiers = 0,
            InputOption = NoOptions);
    QTUITEST_EXPORT bool keyClick  (QObject*,const QByteArray&,int,Qt::KeyboardModifiers = 0,
            InputOption = NoOptions);

    QTUITEST_EXPORT int maximumUiTimeout();

    QTUITEST_EXPORT Qt::Key asciiToKey(char);
    QTUITEST_EXPORT Qt::Key variantToKey(const QVariant &);
    QTUITEST_EXPORT Qt::KeyboardModifiers asciiToModifiers(char);

    QTUITEST_EXPORT QObject* testWidget(QObject*,const char*);

    QTUITEST_EXPORT bool connectFirst   (const QObject*, const char*, const QObject*, const char*);
    QTUITEST_EXPORT bool disconnectFirst(const QObject*, const char*, const QObject*, const char*);

    template<class T> inline T qtuitest_cast_helper(QObject* object, T)
    {
        T ret;
        if ((ret = qobject_cast<T>(object))) {}
        else {
            ret = qobject_cast<T>(QtUiTest::testWidget(object,
                        static_cast<T>(0)->_q_interfaceName()));
        }
        return ret;
    }

    QTUITEST_EXPORT void wait(int);
    QTUITEST_EXPORT bool waitForSignal(QObject*, const char*, int = QtUiTest::maximumUiTimeout(), Qt::ConnectionType = Qt::QueuedConnection);
    QTUITEST_EXPORT bool waitForEvent(QObject*, QEvent::Type, int = QtUiTest::maximumUiTimeout(), Qt::ConnectionType = Qt::QueuedConnection);
    QTUITEST_EXPORT bool waitForEvent(QObject*, QList<QEvent::Type> const&, int = QtUiTest::maximumUiTimeout(), Qt::ConnectionType = Qt::QueuedConnection);

    QTUITEST_EXPORT char keyToAscii(Qt::Key key);
};

template<class T> inline
T qtuitest_cast(const QObject* object)
{
    return QtUiTest::qtuitest_cast_helper<T>(const_cast<QObject*>(object),0);
}

#include "qtuitestwidgetinterface.h"

#endif

