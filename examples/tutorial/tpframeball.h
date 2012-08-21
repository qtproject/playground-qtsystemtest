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

#ifndef TPBALL_H
#define TPBALL_H

#include <QWidget>

#include <QDebug>

class QLabel;

class TPFrameBall : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int ball READ ball WRITE setBall NOTIFY ballChanged USER true)
    Q_PROPERTY(int previousBall READ previousBall WRITE setPreviousBall)
    Q_PROPERTY(int score READ score NOTIFY scoreChanged DESIGNABLE false STORED false)
    Q_PROPERTY(QString display READ display DESIGNABLE false STORED false)
    Q_PROPERTY(bool isStrike READ isStrike DESIGNABLE false STORED false)
    Q_PROPERTY(bool isSpare READ isSpare DESIGNABLE false STORED false)
    Q_PROPERTY(bool isLastFrame READ isLastFrame WRITE setLastFrame)
public:
    explicit TPFrameBall(QWidget *parent = 0);
    
    QString display() const { return m_display; }
    int ball() const { return m_ball; }
    int previousBall() const { return m_previousBall; }
    int score() const { return m_score; }
    bool isStrike() const { return m_previousBall == -1 && score() == 10; }
    bool isSpare() const { return m_previousBall != 10 && score() == 10; }
    bool isLastFrame() const { return m_lastFrame; }
    void setLastFrame(bool value) { m_lastFrame = value; updateScore(); }

public slots:
    void setBall(int);
    void setPreviousBall(int);

signals:
    void ballChanged(int);
    void scoreChanged(int);

protected:
    void mousePressEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);

    void paintEvent(QPaintEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

private slots:
    void updateScore();

private:
    QString m_display;
    int m_score;
    int m_ball;
    int m_previousBall;
    QLabel *m_label;

    bool m_lastFrame;
};

#endif // TPBALL_H
