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

import QtSystemTest 1.0

TestCollection {
    name: 'Input test collection'
    description: 'enter various values for first frame and validate display'

    InputTest {
        name: 'Gutter Ball'
        firstEntry: ['0']
        secondEntry: ['0']

        first: '0'
        second: '0'
        display: '0'
    }
    InputTest {
        name: 'Strike'

        firstEntry: ['1', '0']
        secondEntry: []

        first: 'X'
        second: ''
        display: ''
    }
    InputTest {
        name: 'Three Seven Spare'

        firstEntry: ['3']
        secondEntry: ['7']

        first: '3'
        second: '/'
        display: ''
    }
    InputTest {
        name: 'Three Zero'
        firstEntry: ['3']
        secondEntry: ['0']

        first: '3'
        second: '0'
        display: '3'
    }
    InputTest {
        name: 'Zero Ten Spare'

        firstEntry: ['0']
        secondEntry: ['1', '0']

        first: '0'
        second: '/'
        display: ''
    }
    InputTest {
        name: 'Zero Three'
        firstEntry: ['0']
        secondEntry: ['3']

        first: '0'
        second: '3'
        display: '3'
    }
}
