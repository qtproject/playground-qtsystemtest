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


#include "qinputgenerator_generic_p.h"
#include "qtuitestnamespace.h"

static QWidget *focusWidget()
{
    QTT_TRACE_FUNCTION();
    QWidget *ret = QApplication::focusWidget();
    if (!ret) {
        QWidgetList topLevel = QApplication::topLevelWidgets();
        if (!topLevel.isEmpty())
            return topLevel.first();
    }
    return ret;
}

QInputGeneratorGeneric::QInputGeneratorGeneric(QObject* parent)
    : QInputGenerator(parent),
      currentPos(),
      currentButtons(0)
{
    QTT_TRACE_FUNCTION();
}

QInputGeneratorGeneric::~QInputGeneratorGeneric()
{
    QTT_TRACE_FUNCTION();
}

void QInputGeneratorGeneric::keyPress(Qt::Key key, Qt::KeyboardModifiers mod, bool autoRepeat)
{
    QTT_TRACE_FUNCTION();
    qttDebug() << "keyPress" << key << mod << autoRepeat;
    Q_UNUSED(autoRepeat);
    keyEvent(Press, focusWidget(), key, mod);
}

void QInputGeneratorGeneric::keyRelease(Qt::Key key, Qt::KeyboardModifiers mod)
{
    QTT_TRACE_FUNCTION();
    qttDebug() << "keyRelease" << key << mod;
    keyEvent(Release, focusWidget(), key, mod);
}

void QInputGeneratorGeneric::keyClick(Qt::Key key, Qt::KeyboardModifiers mod)
{
    QTT_TRACE_FUNCTION();
    qttDebug() << "keyClick" << key << mod;
    keyEvent(Click, focusWidget(), key, mod);
}

void QInputGeneratorGeneric::touchPress(const QVariantList& points)
{
    Q_UNUSED(points);
    QTT_TRACE_FUNCTION();
}

void QInputGeneratorGeneric::touchUpdate(const QVariantList& points)
{
    Q_UNUSED(points);
    QTT_TRACE_FUNCTION();
}

void QInputGeneratorGeneric::touchRelease(const QVariantList& points)
{
    Q_UNUSED(points);
    QTT_TRACE_FUNCTION();
}

void QInputGeneratorGeneric::mousePress(const QPoint& pos, Qt::MouseButtons state)
{
    QTT_TRACE_FUNCTION();
    const Qt::MouseButton buttons[] = {Qt::LeftButton, Qt::RightButton, Qt::MidButton, Qt::XButton1, Qt::XButton2};
    QWidget *widget=QApplication::widgetAt(pos);
    if (pos != currentPos) {
        mouseEvent(MouseMove, widget, Qt::NoButton, currentButtons, 0, widget->mapFromGlobal(pos));
        currentPos = pos;
    }
    for (unsigned int i=0; i<sizeof(buttons)/sizeof(Qt::MouseButton); i++) {
        if (state.testFlag(buttons[i])) {
            qttDebug() << "mousePress" << pos << buttons[i] << currentButtons;
            currentButtons |= buttons[i];
            mouseEvent(MousePress, widget, buttons[i], buttons[i], 0, widget->mapFromGlobal(pos));
        }
    }
}

void QInputGeneratorGeneric::mouseRelease(const QPoint& pos, Qt::MouseButtons state)
{
    QTT_TRACE_FUNCTION();
    const Qt::MouseButton buttons[] = {Qt::LeftButton, Qt::RightButton, Qt::MidButton, Qt::XButton1, Qt::XButton2};
    QWidget *widget=QApplication::widgetAt(pos);
    qttDebug() << "widget:" << widget;
    if (pos != currentPos) {
        mouseEvent(MouseMove, widget, Qt::NoButton, currentButtons, 0, widget->mapFromGlobal(pos));
        currentPos = pos;
    }
    for (unsigned int i=0; i<sizeof(buttons)/sizeof(Qt::MouseButton); i++) {
        if (state.testFlag(buttons[i])) {
            qttDebug() << "mouseRelease" << pos << buttons[i] << currentButtons;
            currentButtons ^= buttons[i];
            mouseEvent(MouseRelease, widget, buttons[i], currentButtons, 0, widget->mapFromGlobal(pos));
        }
    }
}

void QInputGeneratorGeneric::mouseClick(const QPoint& pos, Qt::MouseButtons state)
{
    QTT_TRACE_FUNCTION();
    const Qt::MouseButton buttons[] = {Qt::LeftButton, Qt::RightButton, Qt::MidButton, Qt::XButton1, Qt::XButton2};
    QWidget *widget=QApplication::widgetAt(pos);
    if (pos != currentPos) {
        mouseEvent(MouseMove, widget, Qt::NoButton, currentButtons, 0, widget->mapFromGlobal(pos));
        currentPos = pos;
    }
    for (unsigned int i=0; i<sizeof(buttons)/sizeof(Qt::MouseButton); i++) {
        if (state.testFlag(buttons[i])) {
            qttDebug() << "mouseClick" << pos << buttons[i];
            mouseEvent(MouseClick, widget, buttons[i], state, 0, widget->mapFromGlobal(pos));
        }
    }
}

void QInputGeneratorGeneric::mouseDClick(const QPoint& pos, Qt::MouseButtons state)
{
    QTT_TRACE_FUNCTION();
    const Qt::MouseButton buttons[] = {Qt::LeftButton, Qt::RightButton, Qt::MidButton, Qt::XButton1, Qt::XButton2};
    QWidget *widget=QApplication::widgetAt(pos);
    if (pos != currentPos) {
        mouseEvent(MouseMove, widget, Qt::NoButton, currentButtons, 0, widget->mapFromGlobal(pos));
        currentPos = pos;
    }
    for (unsigned int i=0; i<sizeof(buttons)/sizeof(Qt::MouseButton); i++) {
        if (state.testFlag(buttons[i])) {
            qttDebug() << "mouseClick" << pos << buttons[i];
            mouseEvent(MouseDClick, widget, buttons[i], state, 0, widget->mapFromGlobal(pos));
        }
    }
}

void QInputGeneratorGeneric::mouseMove(const QPoint& pos)
{
    QTT_TRACE_FUNCTION();
    qttDebug() << "mouseMove" << pos;
    QWidget *widget=QApplication::widgetAt(pos);
    if (pos != currentPos) {
        currentPos = pos;
    }
    mouseEvent(MouseMove, widget, Qt::NoButton, currentButtons, 0, widget->mapFromGlobal(pos));
}

