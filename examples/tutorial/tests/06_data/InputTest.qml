/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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

import QtQuick 1.1
import QtSystemTest 1.0

TenPinTest {
    name: 'Input Test'
    description: 'Add some ball values and validate displays'

    property variant firstEntry: ['3']
    property variant secondEntry: ['6']

    property string first: '3'
    property string second: '6'
    property string display: '9'
    
    function data() {}

    function setup() {
        startApplication();
        
        data.main = app.items({inherits: 'TPFrame'})[0];
        expect(data.main).not.toBe(null);
        expect(data.main).not.toBe(undefined);

        data.first = data.main.descendants({inherits: 'TPFrameBall'})[0];
        expect(data.first).not.toBe(null);
        data.second = data.main.descendants({inherits: 'TPFrameBall'})[1];
        expect(data.second).not.toBe(null);
    }

    function run() {
        expect(data.first.property('display')).toBe('');
        expect(data.second.property('display')).toBe('');
        expect(data.main.property('display')).toBe('');

        app.mouseClick(data.first);
        firstEntry.forEach(function(item) {
            app.keyPress(item);
        });
        app.mouseClick(data.second);
        secondEntry.forEach(function(item) {
            app.keyPress(item);
        });

        expect(data.first.property('display')).toBe(first);
        expect(data.second.property('display')).toBe(second);
        expect(data.main.property('display')).toBe(display);
    }

    function cleanup() {
        terminateApplication();
    }
}
