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

import QtQuick 2.0
import QtSystemTest 1.0

TestCollection {
    name: 'expect tests'; description: 'A expectation tests'

    Test {
        name: 'toBe'; description: 'to be'

        function run() {
            expect(3).toBe(3);
            expect(2 + 2).toBe(3); // for large values of 3
        }
    }

    Test {
        name: 'not.toBe'; description: 'not to be'

        function run() {
            expect('Claudius').not.toBe('Polonius');
            expect('Claudius').not.toBe('Claudius');
        }
    }

    Test {
        name: 'instanceof'; description: 'expect to be instance of'

        function run() {
            var message = new String('Hello World');
            expect(message).instanceOf(String);
            expect(message).not.instanceOf(Number);
            expect(message).instanceOf(Number);
        }
    }

    Test {
        name: 'between'; description: 'expect to be between'

        function run() {
            expect(2 + 2).toBeBetween(3,5);
            expect(2 + 2).toBeBetween(4,6);
        }
    }

}