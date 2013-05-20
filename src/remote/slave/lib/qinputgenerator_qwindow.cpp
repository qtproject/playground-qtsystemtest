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


#include "qinputgenerator_qwindow_p.h"
#include "qtuitestnamespace.h"

static QWindow *activeWindow()
{
    QTT_TRACE_FUNCTION();
    QWindow *w=0;
    if (!qApp->topLevelWindows().isEmpty())
      w = qApp->topLevelWindows().first();

    return w;
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
    keyEvent(Press, activeWindow(), key, mod);
}

void QInputGeneratorGeneric::keyRelease(Qt::Key key, Qt::KeyboardModifiers mod)
{
    QTT_TRACE_FUNCTION();
    qttDebug() << "keyRelease" << key << mod;
    keyEvent(Release, activeWindow(), key, mod);
}

void QInputGeneratorGeneric::keyClick(Qt::Key key, Qt::KeyboardModifiers mod)
{
    QTT_TRACE_FUNCTION();
    qttDebug() << "keyClick" << key << mod;
    keyEvent(Click, activeWindow(), key, mod);
}

void QInputGeneratorGeneric::touchPress(const QVariantList& points)
{
    QTT_TRACE_FUNCTION();
    QWindow *window=activeWindow();

    touchEvent(TouchPress, window, points);
}

void QInputGeneratorGeneric::touchUpdate(const QVariantList& points)
{
    QTT_TRACE_FUNCTION();
    QWindow *window=activeWindow();
    touchEvent(TouchUpdate, window, points);
}

void QInputGeneratorGeneric::touchRelease(const QVariantList& points)
{
    QTT_TRACE_FUNCTION();
    QWindow *window=activeWindow();
    touchEvent(TouchRelease, window, points);
}

void QInputGeneratorGeneric::mousePress(const QPoint& pos, Qt::MouseButtons state)
{
    QTT_TRACE_FUNCTION();
    const Qt::MouseButton buttons[] = {Qt::LeftButton, Qt::RightButton, Qt::MidButton, Qt::XButton1, Qt::XButton2};
    QWindow *window=activeWindow();
    if (pos != currentPos) {
        mouseEvent(MouseMove, window, Qt::NoButton, currentButtons, 0, window->mapFromGlobal(pos));
        currentPos = pos;
    }
    for (unsigned int i=0; i<sizeof(buttons)/sizeof(Qt::MouseButton); i++) {
        if (state.testFlag(buttons[i])) {
            qttDebug() << "mousePress" << pos << buttons[i] << currentButtons;
            currentButtons |= buttons[i];
            mouseEvent(MousePress, window, buttons[i], buttons[i], 0, window->mapFromGlobal(pos));
        }
    }
}

void QInputGeneratorGeneric::mouseRelease(const QPoint& pos, Qt::MouseButtons state)
{
    QTT_TRACE_FUNCTION();
    const Qt::MouseButton buttons[] = {Qt::LeftButton, Qt::RightButton, Qt::MidButton, Qt::XButton1, Qt::XButton2};
    QWindow *window=activeWindow();
    qttDebug() << "window:" << window;
    if (pos != currentPos) {
        mouseEvent(MouseMove, window, Qt::NoButton, currentButtons, 0, window->mapFromGlobal(pos));
        currentPos = pos;
    }
    for (unsigned int i=0; i<sizeof(buttons)/sizeof(Qt::MouseButton); i++) {
        if (state.testFlag(buttons[i])) {
            qttDebug() << "mouseRelease" << pos << buttons[i] << currentButtons;
            currentButtons ^= buttons[i];
            mouseEvent(MouseRelease, window, buttons[i], currentButtons, 0, window->mapFromGlobal(pos));
        }
    }
}

void QInputGeneratorGeneric::mouseClick(const QPoint& pos, Qt::MouseButtons state)
{
    QTT_TRACE_FUNCTION();
    const Qt::MouseButton buttons[] = {Qt::LeftButton, Qt::RightButton, Qt::MidButton, Qt::XButton1, Qt::XButton2};
    QWindow *window=activeWindow();
    if (pos != currentPos) {
        mouseEvent(MouseMove, window, Qt::NoButton, currentButtons, 0, window->mapFromGlobal(pos));
        currentPos = pos;
    }
    for (unsigned int i=0; i<sizeof(buttons)/sizeof(Qt::MouseButton); i++) {
        if (state.testFlag(buttons[i])) {
            qttDebug() << "mouseClick" << pos << buttons[i];
            mouseEvent(MouseClick, window, buttons[i], state, 0, window->mapFromGlobal(pos));
        }
    }
}

void QInputGeneratorGeneric::mouseDClick(const QPoint& pos, Qt::MouseButtons state)
{
    QTT_TRACE_FUNCTION();
    const Qt::MouseButton buttons[] = {Qt::LeftButton, Qt::RightButton, Qt::MidButton, Qt::XButton1, Qt::XButton2};
    QWindow *window=activeWindow();
    if (pos != currentPos) {
        mouseEvent(MouseMove, window, Qt::NoButton, currentButtons, 0, window->mapFromGlobal(pos));
        currentPos = pos;
    }
    for (unsigned int i=0; i<sizeof(buttons)/sizeof(Qt::MouseButton); i++) {
        if (state.testFlag(buttons[i])) {
            qttDebug() << "mouseClick" << pos << buttons[i];
            mouseEvent(MouseDClick, window, buttons[i], state, 0, window->mapFromGlobal(pos));
        }
    }
}

void QInputGeneratorGeneric::mouseMove(const QPoint& pos)
{
    QTT_TRACE_FUNCTION();
    qttDebug() << "mouseMove" << pos;
    QWindow *window=activeWindow();
    if (pos != currentPos) {
        currentPos = pos;
    }
    mouseEvent(MouseMove, window, Qt::NoButton, currentButtons, 0, window->mapFromGlobal(pos));
}
