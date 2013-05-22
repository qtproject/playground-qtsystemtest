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

QtObject {
    id: logger
    property variant runner: null
    property list<QtObject> __list
    property string path
    default property alias children : logger.__list

    function setup() {
        runner = __runner;
    }

    function locationOf(item) {
        var ret = {
            fileName : '',
            lineNumber: 0
        };

        var loc = runner.locationOf(item);
        if (loc.url === undefined) {
            return ret;
        }
        var url = loc.url.toString();
        ret.fileName = url.substr(url.lastIndexOf('/')+1);
        ret.lineNumber = loc.lineNumber;
        return ret;
    }


    onRunnerChanged: {
        if (runner == undefined || runner == null) return;
        runner.aboutToStartExecution.connect(aboutToStartExecution);
        runner.finishedExecution.connect(finishedExecution);
        runner.testCollectionStart.connect(testCollectionStart);
        runner.testCollectionEnd.connect(testCollectionEnd);

        runner.testStart.connect(testStart);
        runner.testEnd.connect(testEnd);

        runner.methodStart.connect(methodStart);
        runner.methodEnd.connect(methodEnd);
    }

    function aboutToStartExecution() { }
    function finishedExecution() { }

    function testCollectionStart(testCollection) { }
    function testCollectionEnd(testCollection) { }

    function testStart(testCollection) { }
    function testEnd(testCollection) { }

    function methodStart(method) { }
    function methodEnd(method) { }

    function cleanup() { }

    function log(type, event) { }
}
