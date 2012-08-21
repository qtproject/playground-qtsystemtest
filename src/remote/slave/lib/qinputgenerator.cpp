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


#include "qinputgenerator_p.h"

#include <QPoint>
#include <QtSystemTest/QstDebug>

QInputGenerator::QInputGenerator(QObject* parent)
    : QObject(parent)
{
    QTT_TRACE_FUNCTION();
}

QInputGenerator::~QInputGenerator()
{
    QTT_TRACE_FUNCTION();
}

void QInputGenerator::keyPress  (Qt::Key, Qt::KeyboardModifiers, bool) {};
void QInputGenerator::keyRelease(Qt::Key, Qt::KeyboardModifiers) {};
void QInputGenerator::keyClick  (Qt::Key, Qt::KeyboardModifiers) {};

void QInputGenerator::touchPress  (const QVariantList&) {};
void QInputGenerator::touchUpdate  (const QVariantList&) {};
void QInputGenerator::touchRelease  (const QVariantList&) {};

void QInputGenerator::mousePress  (const QPoint&, Qt::MouseButtons) {};
void QInputGenerator::mouseRelease(const QPoint&, Qt::MouseButtons) {};
void QInputGenerator::mouseClick  (const QPoint&, Qt::MouseButtons) {};
void QInputGenerator::mouseDClick (const QPoint&, Qt::MouseButtons) {};
void QInputGenerator::mouseMove   (const QPoint&) {};


Qt::KeyboardModifier const QInputGenerator::AllModifiers[] =
    { Qt::ShiftModifier, Qt::ControlModifier, Qt::AltModifier, Qt::MetaModifier };

Qt::Key QInputGenerator::modifierToKey(Qt::KeyboardModifier mod)
{
    QTT_TRACE_FUNCTION();
    switch (mod) {
        case Qt::ShiftModifier:
            return Qt::Key_Shift;
        case Qt::ControlModifier:
            return Qt::Key_Control;
        case Qt::AltModifier:
            return Qt::Key_Alt;
        case Qt::MetaModifier:
            return Qt::Key_Meta;
        default:
            break;
    }
    return Qt::Key(0);
}
