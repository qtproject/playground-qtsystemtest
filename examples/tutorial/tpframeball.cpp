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

#include "tpframeball.h"
#include <QKeyEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QTextOption>

#include <QDebug>

TPFrameBall::TPFrameBall(QWidget *parent) :
    QWidget(parent),
    m_score(-1),
    m_ball(-1),
    m_previousBall(-1),
    m_lastFrame(true)
{
    setFocusPolicy(Qt::StrongFocus);
}

void TPFrameBall::setBall(int value)
{
    if (value < 0 || value > 10)
        value = -1;
    if (value == m_ball)
        return;
    m_ball = value;
    emit ballChanged(m_ball);
    updateScore();
}

void TPFrameBall::setPreviousBall(int value)
{
    if (value < 0 || value > 10)
        value = -1;
    if (m_lastFrame && value == 10)
        value = -1;
    if (value == m_previousBall)
        return;
    m_previousBall = value;
    updateScore();
}

void TPFrameBall::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    setFocus();
}

void TPFrameBall::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_0:
        if (m_ball == 1)
            setBall(10);
        else
            setBall(0);
        break;
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
        setBall(event->key() - Qt::Key_0);
        break;
    case Qt::Key_Backspace:
        if (m_ball == 10)
            setBall(1);
        else
            setBall(-1);
        break;
    case Qt::Key_X:
        setBall(10);
        break;
    case Qt::Key_Slash:
        if (m_previousBall != -1)
            setBall(10 - m_previousBall);
        break;
    default:
        break;
    }
}

void TPFrameBall::updateScore()
{
    int score = -1;
    if (m_ball != -1 && m_previousBall != -1)
        score = m_ball + m_previousBall;
    else if (m_ball != -1)
        score = m_ball;

    if (m_score != score) {
        m_score = score;
        emit scoreChanged(m_score);
    }

    QString text;
    if (isStrike())
        text = QString::fromLatin1("X");
    else if (isSpare())
        text = QString::fromLatin1("/");
    else if (m_ball == -1)
        text = QString();
    else if (m_score < 0 || m_score > 10)
        text = tr("E", "Error letter, not X or /");
    else
        text = QString::number(m_ball);

    if (text != m_display) {
        m_display = text;
        update();
    }
}

void TPFrameBall::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setClipRect(event->rect());

    QPen pen(Qt::red);
    QBrush brush(hasFocus() ? Qt::blue : Qt::black);
    p.setPen(pen);
    p.setBrush(brush);
    p.drawRect(0, 0, width(), height()-1);

    pen.setColor(Qt::white);
    QFont f = font();
    f.setWeight(QFont::Bold);
    p.setPen(pen);
    p.drawText(0, 0, width(), height(), Qt::AlignHCenter | Qt::AlignVCenter, m_display);
}

void TPFrameBall::focusInEvent(QFocusEvent *)
{
    update();
}

void TPFrameBall::focusOutEvent(QFocusEvent *)
{
    update();
}
