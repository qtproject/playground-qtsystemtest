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

import QstRunner 1.0
import QtSystemTest 1.0
import QtSystemTest.Runner 1.0
import "QstRunner.js" as JsRunner

Runner {
    id: runner

    property variant currentTestItem
    property string currentMethod

    property variant report : ({
        'passed': 0,
        'failed': 0,
        'skipped': 0,
        'untested': 0,
        'error': 0,
    });

    property variant __loggers

    signal aboutToStartExecution()
    signal finishedExecution()
    signal testCollectionStart(variant testCollection)
    signal testCollectionEnd(variant testCollection)

    signal testStart(variant test)
    signal testEnd(variant test)

    signal methodStart(string methodName)
    signal methodEnd(string methodName)

    property variant component: Component {
        id: configComponent
        Config { }
    }



    function __broadcast(fnName, args) {
        for (var i =0; i < __loggers.length; ++i) {
            var logger = __loggers[i];
            if ( ! (fnName in logger) ) {
                throw new Error('function ' + fnName + ' not in ' + logger);
            }
            var f = logger[fnName];
            f.apply(logger, args);
        };
    }

    function logEvent(type, error) {
        __broadcast('log', [type, error]);
    }


    function log(messages) {
        var str = js.core().stringify.apply(null, arguments);
        __broadcast(arguments.callee.name, ['debug', { message: str }]);
    }


    function __setup(args) {
        var x = configComponent.createObject(runner);
        setRootProperty('config', x);
        __loggers = x.loggers;
        Array.prototype.forEach.call(__loggers, function(l) {
            l.setup();
        });
        return true;
    }

    property Js js: Js { }

    function __exec() {
        var result = __testRoot.__validate();
        if (!result.valid) {
            __broadcast('log', ['Error', result.error]);
            return JsRunner.InternalError;
        }
        console.log('Test validation - okay?:  ',  result.valid);

        result = 1 << 6;
        try {
         var x = JsRunner.create(js.core());
         result = x.exec(runner, __testRoot);
        } catch (e) {
            console.log('Error: internal: ', e.toString());
            console.log('Details:')
            for (var k in e) {
                console.log('     ', k, ': ' , e[k]);
            }
        }
        return result;
    }
}
