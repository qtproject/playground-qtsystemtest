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


#ifndef QKEYGENERATOR_P_H
#define QKEYGENERATOR_P_H

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
#include <QObject>
#include <QVariantList>
#include <QtSystemTest/qstglobal.h>

class QPoint;
class QInputGeneratorPrivate;

class QTUITEST_EXPORT QInputGenerator : public QObject
{
Q_OBJECT
public:
    explicit QInputGenerator(QObject* =0);
    virtual ~QInputGenerator();

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
    QInputGeneratorPrivate* d;
    friend class QInputGeneratorPrivate;

    static Qt::Key modifierToKey(Qt::KeyboardModifier);
    static Qt::KeyboardModifier const AllModifiers[4];
};

#endif

