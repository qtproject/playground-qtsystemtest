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

#ifndef TPFRAME_H
#define TPFRAME_H

#include <QFrame>

namespace Ui {
class TPFrame;
}

class TPFrame : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(int first READ first DESIGNABLE false STORED false)
    Q_PROPERTY(int second READ second DESIGNABLE false STORED false)
    Q_PROPERTY(int third READ third DESIGNABLE false STORED false)
    Q_PROPERTY(int score READ score NOTIFY scoreChanged DESIGNABLE false STORED false)
    Q_PROPERTY(QString display READ display DESIGNABLE false STORED false)
    Q_PROPERTY(int runningTotal READ runningTotal NOTIFY runningTotalChanged DESIGNABLE false STORED false)

    Q_PROPERTY(TPFrame *previousFrame READ previousFrame WRITE setPreviousFrame DESIGNABLE false STORED false)
    Q_PROPERTY(TPFrame *nextFrame READ nextFrame WRITE setNextFrame DESIGNABLE false STORED false)

public:
    explicit TPFrame(QWidget *parent = 0);
    ~TPFrame();

    int first() const;
    int second() const;
    int third() const;
    int score() const { return m_score; }
    int runningTotal() const;
    QString display() const;

    TPFrame *previousFrame() const { return m_previous; }
    TPFrame *nextFrame() const { return m_next; }

    void setPreviousFrame(TPFrame *);
    void setNextFrame(TPFrame *);

public slots:
    void reset();

signals:
    void runningTotalChanged(int);
    void scoreChanged(int);
    void ballsChanged();

private slots:
    void updateScore();
    void updateTotal();

private:
    Ui::TPFrame *ui;
    TPFrame *m_previous;
    TPFrame *m_next;
    int m_score;
};

#endif // TPFRAME_H
