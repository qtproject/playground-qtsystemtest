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

Js.include_check(Qtt === undefined, 'Exception.js already included');
Js.require('Class.js');
Js.require('Console.js');

var Qtt = function(){
    var QttDebug = Console.topic('Qtt');

    var Status = {
        Error:      'error',
        Passed:     'passed',
        Failed:     'failed',
        Skipped:    'skipped',
        Aborted:    'aborted',
        Untested:   'untested',
    };

    var QttError = Class.create(function QttError(msg, discardLevel) {
        discardLevel = (discardLevel || 0 ) + 2;
        this.name = arguments.callee.name;
        this.stack = Qt.qst.stack();
        Js.discardCallSites.call(this, discardLevel);
        return {
            status : Status.Error,
            type: 'QttError',
            message: msg,
            __addStack : function(){},
            discardCallSites: Js.discardCallSites,
            toString: function() { return this.message; }
        };
    });

    var Skip = QttError.extend(function Skip(msg, discardLevel) {
        discardLevel = (discardLevel || 0 ) + 2;
        this._super(msg, discardLevel);
        this.type = 'Skip';
        this.status = Status.Skipped;
        return { };
    });

    var Abort = QttError.extend(function Abort(msg, discardLevel) {
        discardLevel = (discardLevel || 0 ) + 2;
        this._super(msg, discardLevel);
        this.type = 'Abort';
        this.status = Status.Aborted;
        return { };
    });

    var Fail = QttError.extend(function Fail(msg, discardLevel) {
        discardLevel = (discardLevel || 0 ) + 2;
        this._super(msg, discardLevel);
        this.type = 'Fail';
        this.status =Status.Failed;
        return { };
    });


    return {
        Error: QttError,
        Skip: Skip,
        Fail: Fail,
        Abort: Abort,
        Status: Status
    };
}();
