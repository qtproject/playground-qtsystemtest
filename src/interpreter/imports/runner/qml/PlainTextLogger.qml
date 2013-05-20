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

import QtQuick 1.1
import QtSystemTest.Runner 1.0

Logger {
    id: plaintextLogger

    property PlainTextWriter writer: PlainTextWriter {}

    property int indentLevel : 0
    property bool writeFinished : true
    property string indentString: '  '
    property int maxNameLen: 60

    property variant status : ({
    });

    function aboutToStartExecution() {
        writer.filePath = plaintextLogger.path;
    }

    function testCollectionStart(testCollection) {
        var loc = locationOf(testCollection);
        logString( 'Collection', pad("'"+ testCollection.name + "'  ", maxNameLen - 2*indentLevel)
                + '   : ' + loc.fileName + '('+ loc.lineNumber +')');
        indentLevel++;
    }

    function testStart(test) { var loc = locationOf(test);
        logString( 'Test', pad("'"+test.name + "'  ", maxNameLen - 2*indentLevel)
                + '   : ' + loc.fileName + '('+ loc.lineNumber +')');
        indentLevel++;
    }


    function methodStart(method) {
        logString('Running', method + '()');
        writeFinished = false;
        indentLevel++;
    }

    function methodEnd(method) {
        indentLevel--;
        if ( writeFinished ) {
            logString('Finished', method + '()');
        }
    }

    function testEnd(test) {
        indentLevel--;
        var statusMap = {
            error:  { text: 'ERROR', foreground: PlainTextWriter.White, background: PlainTextWriter.Red },
            passed: { text: 'passed', foreground: PlainTextWriter.Green, background: PlainTextWriter.DefaultColor },
            skipped: { text: 'skipped', foreground: PlainTextWriter.Yellow, background: PlainTextWriter.DefaultColor },
            failed:  { text: 'FAILED', foreground: PlainTextWriter.Red, background: PlainTextWriter.DefaultColor },
            untested: { text: 'UNTESTED', foreground: PlainTextWriter.LightRed, background: PlainTextWriter.DefaultColor }
        };

        var result = test.__status.toLowerCase();
        var status = statusMap[result];
        logString( 'Result', pad("'"+test.name + "'  ", maxNameLen - 2*indentLevel, '.')
            + '   : [' + writer.colorize(status.text, status.foreground, status.background) + ']');
    }

    function testCollectionEnd(testCollection) {
        indentLevel--;
        logString( 'Collection', testCollection.name );
    }


    function finishedExecution() {
        write('-'.repeat(80));
        var report = runner.report;
        var totalTests = 0;
        var keys = ['error', 'passed', 'skipped', 'failed', 'untested'];
        keys.forEach(function(s) {
            totalTests += report[s];
        });
        write(pad('Total Tests', 15) + ' :' + totalTests);

        keys.forEach(function(s) {
            if (report[s] > 0 ) {
                write(pad(s, 15) + ' :' + report[s]);
            }
        });
        write('-'.repeat(80));
    }


    function log(type, event) {
        var str = event.formatted || event.message;
        if (event.stack) {
            event.stack.forEach(function(callSite) {
                var trace = Qt.qst.relativeToCurrentPath(callSite.fileName)
                            + '(' + callSite.lineNumber +') : '
                            + callSite.functionName;
                str += '\n';
                str += trace;
            });
        }
        logString(type, str);
    }

    function logString(type, str) {
        writeFinished = true;
        write(pad(type, 10) + ' : ' + str);
    }

    function write(str) {
        var strs = str.split('\n');
        var prefix = '[' + Date.timestamp() + ']'+ indentString.repeat(indentLevel) + ' ';
        for (var i = 0; i < strs.length; ++i) {
            writer.writeLine(prefix + strs[i]);
        }
    }

    function pad(text, len, using) {
        using = using || ' '
        var padding = text.length < len ? using.repeat(len - text.length) : '';
        return text + padding;
    }
}
