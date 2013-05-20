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

import QtSystemTest 1.0
import QtSystemTest.Runner 1.0

Logger {
    id: xmlLogger
    property XmlWriter writer: XmlWriter {}

    property Js js : Js{}
    function store(){}

    function aboutToStartExecution() {
        store.timerStack = [];
        writer.filePath = xmlLogger.path;
        writer.writeStartDocument();
        writer.writeStartElement('systemtestrun');
        store.timerStack.push(+new Date);
    }

    function finishedExecution() {
        var duration = (+new Date) - store.timerStack.pop();

        writer.writeStartElement('report');

        var report = runner.report;
        var totalTests = 0;
        var keys = ['passed', 'skipped', 'untested', 'failed', 'error'];
        keys.forEach(function(s) {
            totalTests += report[s];
            writer.writeTextElement(s, report[s].toString());
        });
        writer.writeTextElement('total', totalTests.toString());
        writer.writeTextElement('duration', duration);
        writer.writeEndElement(); // report
        writer.writeEndElement(); // systemtestrun
        writer.writeEndDocument();
    }

    function testCollectionStart(testCollection) {
        var loc = locationOf(testCollection);
        writer.writeStartElement('collection');
        writer.writeAttribute('name', testCollection.name);
        writer.writeAttribute('file', loc.fileName);
        writer.writeAttribute('line', loc.lineNumber);
        writer.writeAttribute('timestamp', (new Date).toISOString());
        store.timerStack.push(+new Date);
    }

    function testCollectionEnd(testCollection) {
        var duration = (+new Date) - store.timerStack.pop();
        writer.writeStartElement('report');
        writer.writeTextElement('duration', duration);
        writer.writeEndElement();
        writer.writeEndElement();
    }

    function testStart(test) {
        var loc = locationOf(test);
        writer.writeStartElement('test');
        writer.writeAttribute('name', test.name);
        writer.writeAttribute('file', loc.fileName);
        writer.writeAttribute('line', loc.lineNumber);
        writer.writeAttribute('timestamp', (new Date).toISOString());
        store.timerStack.push(+new Date);
    }

    function testEnd(test) {
        var duration = (+new Date) - store.timerStack.pop();
        writer.writeStartElement('report');
        writer.writeTextElement('status', test.__status.toLowerCase());
        writer.writeTextElement('duration', duration);
        writer.writeEndElement();
        writer.writeEndElement();
    }

    function methodStart(method) {
        writer.writeStartElement('method');
        writer.writeAttribute('name', method);
    }

    function methodEnd(method) {
        writer.writeEndElement();
    }

    function writeFrame(callSite) {
        var path = Qt.qst.relativeToCurrentPath(callSite.fileName);
        writer.writeStartElement('frame');
        writer.writeAttribute('file', path);
        writer.writeAttribute('line', callSite.lineNumber);
        writer.writeCharacters(callSite.functionName);
        writer.writeEndElement();
    }

    function writeEventContents(event)
    {
        var Core = js.core();
        var stringify = Core.stringify.nothrow();
        writer.writeTextElement('reason', event.message);
        if (event.stack) {
            writer.writeStartElement('stack');
            event.stack.forEach(writeFrame.bind(this));
            writer.writeEndElement();
        }
        if (event.meta) {
            writer.writeStartElement('meta')
            for(var part in event.meta) {
                var asString = stringify(event.meta[part]);
                writer.writeStartElement('property');
                writer.writeAttribute('name', part);
                writer.writeCharacters(stringify.failed ? 'opaque object' : asString);
                writer.writeEndElement();
            }
            writer.writeEndElement();
        }
    }

    function log(type, event) {
        var ltype = type.toLowerCase();
        switch(ltype) {
        case 'passed':
        case 'skipped':
        case 'failed':
        case 'error':
        case 'untested':
            writer.writeStartElement(ltype);
            writeEventContents(event);
            writer.writeEndElement();
            break;
        default:
            writer.writeStartElement('note');
            writer.writeAttribute('type', ltype);
            writeEventContents(event);
            writer.writeEndElement();
            break;
        }
    }

}
