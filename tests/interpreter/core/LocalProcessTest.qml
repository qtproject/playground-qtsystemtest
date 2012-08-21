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

Test {
    name: 'process'; description: name
    property int timeout: 3000
    property LocalProcess process: LocalProcess {
        outputMode: LocalProcess.PropertyOutput
    }

    function run() {
        process.start("bc");
        process.write("p = 3 + 4\n");
        process.write("p\n");
        expect(process.waitForReadyRead(timeout)).toBe(true);
        expect(process.standardOutput).toBe('7\n')
        process.write("2 * p\n")
        process.closeWriteChannel();
        expect(process.waitForFinished(timeout)).toBe(true);

        expect(process.standardOutput).toBe('7\n14\n')
        expect(process.exitCode).toBe(0);
    }
}
