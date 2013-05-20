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

#include "tpframe.h"
#include "ui_tpframe.h"

#include <QDebug>

TPFrame::TPFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TPFrame),
    m_previous(0),
    m_next(0),
    m_score(-1)
{
    ui->setupUi(this);
    connect(ui->firstBall, SIGNAL(ballChanged(int)), ui->secondBall, SLOT(setPreviousBall(int)));

    connect(ui->firstBall, SIGNAL(scoreChanged(int)), this, SIGNAL(ballsChanged()));
    connect(ui->secondBall, SIGNAL(scoreChanged(int)), this, SIGNAL(ballsChanged()));
    connect(ui->thirdBall, SIGNAL(scoreChanged(int)), this, SIGNAL(ballsChanged()));

    connect(this, SIGNAL(ballsChanged()), this, SLOT(updateScore()));
}

TPFrame::~TPFrame()
{
    delete ui;
}

int TPFrame::first() const
{
    return ui->firstBall->ball();
}

int TPFrame::second() const
{
    if (m_next)
        return ui->firstBall->isStrike() ? m_next->first() : ui->secondBall->ball();
    return ui->secondBall->ball();
}

int TPFrame::third() const
{
    if (m_next)
        return ui->firstBall->isStrike() ? m_next->second() : m_next->first();
    return ui->thirdBall->ball();
}

int TPFrame::runningTotal() const
{
    if (m_previous) {
        int prevTotal = m_previous->runningTotal();
        if (prevTotal != -1 && m_score != -1)
            return prevTotal + m_score;
        return -1;
    }
    return m_score;
}

void TPFrame::setPreviousFrame(TPFrame *frame)
{
    if (m_previous)
        disconnect(m_previous, 0, this, 0);

    m_previous = frame;

    if (m_previous) {
        connect(m_previous, SIGNAL(runningTotalChanged(int)), this, SLOT(updateTotal()));
        updateTotal();
    }
}

void TPFrame::setNextFrame(TPFrame *frame)
{
    if (m_next)
        disconnect(m_next, 0, this, 0);

    m_next = frame;

    if (m_next) {
        ui->firstBall->setLastFrame(false);
        ui->secondBall->setLastFrame(false);
        ui->thirdBall->setLastFrame(false);
        ui->thirdBall->hide();
        connect(m_next, SIGNAL(ballsChanged()), this, SIGNAL(ballsChanged()));
    } else {
        ui->firstBall->setLastFrame(true);
        ui->secondBall->setLastFrame(true);
        ui->thirdBall->setLastFrame(true);
        ui->thirdBall->show();
    }
}

void TPFrame::reset()
{
    ui->firstBall->setBall(-1);
    ui->secondBall->setBall(-1);
    ui->thirdBall->setBall(-1);
}

void TPFrame::updateScore()
{
    int score = -1;
    int f = first();
    int s = second();
    int t = third();
    if (ui->firstBall->isStrike()) {
        if (s != -1 && t != -1)
            score = 10 + s + t;
    } else if (ui->secondBall->isSpare()) {
        if (t != -1)
            score = 10 + t;
    } else {
        if (f != -1 && s != -1)
            score = f + s;
    }
    if (score == m_score)
        return;
    m_score = score;
    emit scoreChanged(m_score);
    updateTotal();
}

void TPFrame::updateTotal()
{
    int total = runningTotal();
    emit runningTotalChanged(total);
    ui->frameDisplay->setText(total == -1 ? QString() : QString::number(total));
}

QString TPFrame::display() const
{
    return ui->frameDisplay->text();
}
