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


#ifndef QINPUTGENERATORGENERIC_P_H
#define QINPUTGENERATORGENERIC_P_H

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
#include <QWidget>
#include <QMouseEvent>
#include <QPoint>
#include <QPointer>
#include <QApplication>
#include <QtSystemTest/qstglobal.h>
#include <qinputgenerator_p.h>
#include <QtSystemTest/QstDebug>
#include <QtTest/qtestspontaneevent.h>

enum MouseAction { MousePress, MouseRelease, MouseClick, MouseDClick, MouseMove };

static void mouseEvent(MouseAction action, QWidget *widget, Qt::MouseButton button, Qt::MouseButtons buttons,
                        Qt::KeyboardModifiers stateKey, QPoint pos)
{
    QTT_TRACE_FUNCTION();
    if (pos.isNull())
        pos = widget->rect().center();

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

enum KeyAction { Press, Release, Click };

static void simulateEvent(QWidget *widget, bool press, int code,
                          Qt::KeyboardModifiers modifier, QString text, bool repeat)
{
    QTT_TRACE_FUNCTION();
    QKeyEvent a(press ? QEvent::KeyPress : QEvent::KeyRelease, code, modifier, text, repeat);
    QSpontaneKeyEvent::setSpontaneous(&a);
    if (!qApp->notify(widget, &a))
        qWarning() << "Keyboard event not accepted by receiving widget";
}

static void sendKeyEvent(KeyAction action, QWidget *widget, Qt::Key code,
                          QString text, Qt::KeyboardModifiers modifier)
{
    QTT_TRACE_FUNCTION();
    if (!widget)
        widget = QWidget::keyboardGrabber();
    if (!widget) {
        if (QWidget *apw = QApplication::activePopupWidget())
            widget = apw->focusWidget() ? apw->focusWidget() : apw;
        else
            widget = QApplication::focusWidget();
    }
    if (!widget)
        widget = QApplication::activeWindow();

    if (action == Click) {
        QPointer<QWidget> ptr(widget);
        sendKeyEvent(Press, widget, code, text, modifier);
        if (!ptr) {
            // if we send key-events to embedded widgets, they might be destroyed
            // when the user presses Return
            return;
        }
        sendKeyEvent(Release, widget, code, text, modifier);
        return;
    }

    bool repeat = false;

    if (action == Press) {
        if (modifier & Qt::ShiftModifier)
            simulateEvent(widget, true, Qt::Key_Shift, 0, QString(), false);

        if (modifier & Qt::ControlModifier)
            simulateEvent(widget, true, Qt::Key_Control, modifier & Qt::ShiftModifier, QString(), false);

        if (modifier & Qt::AltModifier)
            simulateEvent(widget, true, Qt::Key_Alt,
                          modifier & (Qt::ShiftModifier | Qt::ControlModifier), QString(), false);
        if (modifier & Qt::MetaModifier)
            simulateEvent(widget, true, Qt::Key_Meta, modifier & (Qt::ShiftModifier
                                                                  | Qt::ControlModifier | Qt::AltModifier), QString(), false);
        simulateEvent(widget, true, code, modifier, text, repeat);
    } else if (action == Release) {
        simulateEvent(widget, false, code, modifier, text, repeat);

        if (modifier & Qt::MetaModifier)
            simulateEvent(widget, false, Qt::Key_Meta, modifier, QString(), false);
        if (modifier & Qt::AltModifier)
            simulateEvent(widget, false, Qt::Key_Alt, modifier &
                          (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier), QString(), false);

        if (modifier & Qt::ControlModifier)
            simulateEvent(widget, false, Qt::Key_Control,
                          modifier & (Qt::ShiftModifier | Qt::ControlModifier), QString(), false);

        if (modifier & Qt::ShiftModifier)
            simulateEvent(widget, false, Qt::Key_Shift, modifier & Qt::ShiftModifier, QString(), false);
    }
}

// Convenience function
static void sendKeyEvent(KeyAction action, QWidget *widget, Qt::Key code,
                          char ascii, Qt::KeyboardModifiers modifier)
{
    QTT_TRACE_FUNCTION();
    QString text;
    if (ascii)
        text = QString(QChar::fromLatin1(ascii));
    sendKeyEvent(action, widget, code, text, modifier);
}

inline static void keyEvent(KeyAction action, QWidget *widget, Qt::Key key,
                            Qt::KeyboardModifiers modifier = Qt::NoModifier)
{
    QTT_TRACE_FUNCTION();
    if (modifier & Qt::ShiftModifier)
        sendKeyEvent(action, widget, key, QChar(key).toUpper().unicode(), modifier);
    else
        sendKeyEvent(action, widget, key, QChar(key).toLower().unicode(), modifier);
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
    QPoint           currentPos;
    Qt::MouseButtons currentButtons;

};

#endif

