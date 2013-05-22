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


#ifndef QINPUTGENERATORQWINDOW_P_H
#define QINPUTGENERATORQWINDOW_P_H

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

#include <QtGlobal>
#include <QMouseEvent>
#include <QPoint>
#include <QPointer>
#include <QApplication>
#include <QtSystemTest/qstglobal.h>
#include <qinputgenerator_p.h>
#include <qtuitestnamespace.h>
#include <QtTest/qtestspontaneevent.h>
#include <QtGui/qwindowsysteminterface_qpa.h>
#include <QtSystemTest/QstDebug>

enum MouseAction { MousePress, MouseRelease, MouseClick, MouseDClick, MouseMove };
enum TouchAction { TouchPress, TouchRelease, TouchUpdate };

static void mouseEvent(MouseAction action, QWindow *widget, Qt::MouseButton button, Qt::MouseButtons buttons,
                        Qt::KeyboardModifiers stateKey, QPoint pos)
{
    QTT_TRACE_FUNCTION();
    if (pos.isNull())
        pos = widget->geometry().center();

    if (action == MouseClick) {
        mouseEvent(MousePress, widget, button, buttons, stateKey, pos);
        mouseEvent(MouseRelease, widget, button, 0, stateKey, pos);
        return;
    }

    stateKey &= static_cast<unsigned int>(Qt::KeyboardModifierMask);

    QMouseEvent me(QEvent::User, QPoint(), Qt::LeftButton, button, stateKey);
    switch (action)
    {
        case MousePress:
            me = QMouseEvent(QEvent::MouseButtonPress, pos, widget->mapToGlobal(pos), button, buttons, stateKey);
            break;
        case MouseRelease:
            me = QMouseEvent(QEvent::MouseButtonRelease, pos, widget->mapToGlobal(pos), button, buttons, stateKey);
            break;
        case MouseDClick:
            me = QMouseEvent(QEvent::MouseButtonDblClick, pos, widget->mapToGlobal(pos), button, buttons, stateKey);
            break;
        case MouseMove:
              me = QMouseEvent(QEvent::MouseMove, pos, widget->mapToGlobal(pos), Qt::NoButton, buttons, stateKey);
              break;
        default:
              break;
    }
    QSpontaneKeyEvent::setSpontaneous(&me);
    if (!qApp->notify(widget, &me)) {
        static const char *mouseActionNames[] =
            { "MousePress", "MouseRelease", "MouseClick", "MouseDClick", "MouseMove" };
        QString warning = QString::fromLatin1("Mouse event \"%1\" not accepted by receiving widget");
        qWarning() << warning.arg(QString::fromLatin1(mouseActionNames[static_cast<int>(action)]));
    }

}

static QWindowSystemInterface::TouchPoint touchPoint(const QTouchEvent::TouchPoint &pt)
{
    QWindowSystemInterface::TouchPoint p;
    p.id = pt.id();
    p.flags = pt.flags();
    p.normalPosition = pt.normalizedPos();
    p.area = pt.screenRect();
    p.pressure = pt.pressure();
    p.state = pt.state();
    p.velocity = pt.velocity();
    p.rawPositions = pt.rawScreenPositions();
    return p;
}

static QList<struct QWindowSystemInterface::TouchPoint> touchPointList(const QList<QTouchEvent::TouchPoint> &pointList)
{
    QList<struct QWindowSystemInterface::TouchPoint> newList;

    Q_FOREACH (QTouchEvent::TouchPoint p, pointList)
    {
        newList.append(touchPoint(p));
    }
    return newList;
}

static void touchEvent(TouchAction action, QWindow *widget, const QVariantList &points)
{
    QTT_TRACE_FUNCTION();

    const QTouchDevice *dev = 0;
    QList<const QTouchDevice *> devices = QTouchDevice::devices();
    if (devices.size())
        dev = devices.at(0);

    QList<QTouchEvent::TouchPoint> newList;

    for (int i = 0; i < points.size(); ++i) {
        QTouchEvent::TouchPoint p(i);
        p.setPos(points[i].toPoint());
        p.setScreenPos(widget->mapToGlobal(points[i].toPoint()));

        switch (action)
        {
            case TouchPress:
                p.setState(Qt::TouchPointPressed);
                break;
            case TouchRelease:
                p.setState(Qt::TouchPointReleased);
                break;
            default:
                p.setState(Qt::TouchPointMoved);
                break;
        }

        newList.append(p);
    }

    QWindowSystemInterface::handleTouchEvent(widget, const_cast<QTouchDevice*>(dev), touchPointList(newList));
}

enum KeyAction { Press, Release, Click };

static void simulateEvent(QWindow *window, bool press, int code,
                          Qt::KeyboardModifiers modifier, QString text, bool repeat, int delay=-1)
{
    QTT_TRACE_FUNCTION();
    QEvent::Type type;
    type = press ? QEvent::KeyPress : QEvent::KeyRelease;
    QWindowSystemInterface::handleKeyEvent(window, type, code, modifier, text, repeat, delay);
}

static void sendKeyEvent(KeyAction action, QWindow *window, Qt::Key code,
                          QString text, Qt::KeyboardModifiers modifier, int delay=-1)
{
    QTT_TRACE_FUNCTION();
    if (action == Click) {
        sendKeyEvent(Press, window, code, text, modifier, delay);
        sendKeyEvent(Release, window, code, text, modifier, delay);
        return;
    }

    bool repeat = false;

    if (action == Press) {
        if (modifier & Qt::ShiftModifier)
            simulateEvent(window, true, Qt::Key_Shift, 0, QString(), false, delay);

        if (modifier & Qt::ControlModifier)
            simulateEvent(window, true, Qt::Key_Control, modifier & Qt::ShiftModifier, QString(), false, delay);

        if (modifier & Qt::AltModifier)
            simulateEvent(window, true, Qt::Key_Alt,
                          modifier & (Qt::ShiftModifier | Qt::ControlModifier), QString(), false, delay);
        if (modifier & Qt::MetaModifier)
            simulateEvent(window, true, Qt::Key_Meta, modifier & (Qt::ShiftModifier
                                                                  | Qt::ControlModifier | Qt::AltModifier), QString(), false, delay);
        simulateEvent(window, true, code, modifier, text, repeat, delay);
    } else if (action == Release) {
        simulateEvent(window, false, code, modifier, text, repeat, delay);

        if (modifier & Qt::MetaModifier)
            simulateEvent(window, false, Qt::Key_Meta, modifier, QString(), false, delay);
        if (modifier & Qt::AltModifier)
            simulateEvent(window, false, Qt::Key_Alt, modifier &
                          (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier), QString(), false, delay);

        if (modifier & Qt::ControlModifier)
            simulateEvent(window, false, Qt::Key_Control,
                          modifier & (Qt::ShiftModifier | Qt::ControlModifier), QString(), false, delay);

        if (modifier & Qt::ShiftModifier)
            simulateEvent(window, false, Qt::Key_Shift, modifier & Qt::ShiftModifier, QString(), false, delay);
    }
}

static void sendKeyEvent(KeyAction action, QWindow *window, Qt::Key code,
                          char ascii, Qt::KeyboardModifiers modifier, int delay=-1)
{
    QTT_TRACE_FUNCTION();
    QString text;
    if (ascii)
        text = QString(QChar::fromLatin1(ascii));
    sendKeyEvent(action, window, code, text, modifier, delay);
}

inline static void keyEvent(KeyAction action, QWindow *window, char ascii,
                            Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)
{
    QTT_TRACE_FUNCTION();
    sendKeyEvent(action, window, QtUiTest::asciiToKey(ascii), ascii, modifier, delay);
}

inline static void keyEvent(KeyAction action, QWindow *window, Qt::Key key,
                            Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)
{
    QTT_TRACE_FUNCTION();
    if (modifier & Qt::ShiftModifier)
        sendKeyEvent(action, window, key, QChar(key).toUpper().unicode(), modifier, delay);
    else
        sendKeyEvent(action, window, key, QChar(key).toLower().unicode(), modifier, delay);
}


class QTUITEST_EXPORT QInputGeneratorGeneric : public QInputGenerator
{
Q_OBJECT
public:
    explicit QInputGeneratorGeneric(QObject* =0);
    virtual ~QInputGeneratorGeneric();

    virtual void keyPress  (Qt::Key, Qt::KeyboardModifiers, bool=false);
    virtual void keyRelease(Qt::Key, Qt::KeyboardModifiers);
    virtual void keyClick  (Qt::Key, Qt::KeyboardModifiers);

    virtual void touchPress  (const QVariantList&);
    virtual void touchUpdate (const QVariantList&);
    virtual void touchRelease(const QVariantList&);

    virtual void mousePress  (const QPoint&, Qt::MouseButtons);
    virtual void mouseRelease(const QPoint&, Qt::MouseButtons);
    virtual void mouseClick  (const QPoint&, Qt::MouseButtons);
    virtual void mouseDClick (const QPoint&, Qt::MouseButtons);
    virtual void mouseMove   (const QPoint&);

protected:
    void registerDevice();

    QPoint           currentPos;
    Qt::MouseButtons currentButtons;
    QTouchDevice *device;
};

#endif

