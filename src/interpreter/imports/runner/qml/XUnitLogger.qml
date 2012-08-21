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
import QtSystemTest.Runner 1.0

Logger {
    id: xunitLogger

    property var writer: XUnitWriter {
        filePath: xunitLogger.path
    }

    property var tests: []
    property var currentTest
    property var msecsMarker: 0
    property var collectionStack: 0
    property var lastCollection: '';

    function aboutToStartExecution() {
        writer.writeStartDocument();
        writer.writeStartTestSuiteSet();
    }

    function finishedExecution() {
        writer.writeEndTestSuiteSet();
        writer.writeEndDocument();
    }

    function testCollectionStart(testCollection) {
        if (!collectionStack) {
            tests = [];
            msecsMarker = new Date;
        }
        lastCollection = testCollection.name;
        collectionStack++;
    }

    function testCollectionEnd(testCollection) {
        collectionStack--;
        if (collectionStack)
            return;

        var Status = Core.Qtt.Status;

        var passed = 0;
        var failed = 0;
        var skipped = 0;
        var errors = 0;
        var i;
        var test;
        for (i = 0; i < tests.length; i++) {
            test = tests[i];
            switch(test.state) {
            case Status.Passed:
                passed++;
                break;
            case Status.Failed:
                failed++;
                break;
            default:
            case Status.Skipped:
            case Status:Untested:
                skipped++;
                break;
            case Status.Error:
                errors++;
                break;
            }
        }
        writer.writeStartTestSuite(
                    testCollection.name,
                    msecsMarker,
                    'localhost',
                    passed + failed + skipped + errors,
                    failed,
                    errors,
                    skipped,
                    (+(new Date()) - msecsMarker.getTime())/1000);
        for (i = 0; i < tests.length; i++) {
            test = tests[i];
            writer.writeStartTestCase(test.classname, test.name, test.duration);
            var message;
            if (test.messages.length > 0) {
                message = test.messages[test.messages.length-1];
                var messageWritten;
                switch(message.type) {
                case Status.Error:
                    writer.writeError(message.type, message.message);
                    break;
                case Status.Failed:
                    writer.writeFailure(message.type, message.message);
                    break;
                case Status.Skipped:
                case Status.Untested:
                    writer.writeSkipped(message.type, message.message);
                    break;
                default:
                    break;
                }
            }

            writer.writeEndTestCase();
        }
        writer.writeEndTestSuite();
    }

    function testStart(test) {
        currentTest =  {
            state: Core.Qtt.Status.Passed,
            time: +(new Date()),
            classname: lastCollection,
            name: test.name,
            messages: []
        };
    }
    function testEnd(test) {
        currentTest.duration = (+(new Date()) - currentTest.time)/1000;
        currentTest.state = test.__status
        if (test.__status === Core.Qtt.Status.Untested) {
            currentTest.messages.push({type: test.__status, message: 'No verifiable steps performed'});

        }

        tests.push(currentTest);

    }

    function cleanup() { }

    function log(type, msg) {
        currentTest.messages.push({type: type, message: msg});
    }
}
