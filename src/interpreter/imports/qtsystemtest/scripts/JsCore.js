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

if( typeof(Js) !== 'undefined') {
    Js.include_check(Js === undefined, 'JsCore.js already included');
}

Qt.include('Datatype.js')
Qt.include('Functional.js');

var Js = function() {

    var Core = {
        Error: {
            maxStack: 17
        }
    };

//    var DebugEnabled = true;
    var DebugEnabled = false;
    function _debug() {
        var str = '',
            arg,
            i,
            now,
            timestamp;

        if (!DebugEnabled) {
            return;
        }

        for (i =0; i<arguments.length; ++i) {
            arg = arguments[i]
            if (arg != null) {
                str += arg.toString();
            } else {
                str += 'undefined'
            }
        }
        console.log(Date.timestamp(), 'JsCore[Debug]:', str);
    }

    function discardCallSites(num) {
        num = num || 1;
        if (!this.stack) return;
        var stack = this.stack;
        if( stack.length < num ) {
            _debug(' Error: insufficient stack depth');
            throw new Error('Insufficient stack depth');
        }
        while ( num-- > 0 ) {
            this.stack.shift();
        }
    }

    var JsError = (function() {
        function JsError(msg) {
            Error.call(this);
            this.message = msg;
            this.name = arguments.callee.name;
            this.stack = Qt.qst.stack();
            this._discardCallSites();
        }
        JsError.prototype = new Error();
        JsError.prototype._discardCallSites = discardCallSites;
        return JsError;
    })();

    var IncludeError = (function() {
        function IncludeError(msg) {
            JsError.call(this, msg);
            _debug(' Creating include error :', this.message);
            this.name = arguments.callee.name;
            this._discardCallSites();
        }

        IncludeError.prototype = new JsError();
        IncludeError.prototype.constructor = IncludeError;
        return IncludeError;
    })();

    function require(filename) {
        require.level = require.level || 0;

        function repeat(c, t){
            var str = '';
            for(var i=0; i< t; ++i){
                str += c;
            }
            return str;
        }
        var spaces =  repeat(' ', require.level*4);
        _debug('including ....        '+ spaces + (require.level !==0? '+--': '') + '[', filename, ']');

        require.level++;
        try {
            var include = Qt.include(filename);
        } catch(e) {
            console.log('Exception caught loading', filename, e.message);
            console.log('Stack', e.stack.join('\n'));
        }

        if( include && include.status === include.EXCEPTION) {
            var ex  = include.exception;
            if (!(ex instanceof IncludeError) ) {
                console.log('Exception including : ', filename, ': ',  ex.message, ex.filename);
                if ( ex.stack instanceof Array) {
                    ex.stack.forEach( function(cs) {
                        console.log( cs.fileName, ':', s.lineNumber);
                    });
                }
                throw ex;
            } else {
                _debug('Exception including : ', filename, " exception: ", ex);
            }
        }
        //          _debug('including ....        [', filename, ']done');
        require.level--;
    }

    function include_check(condition, msg) {
        if(!condition) {
            throw new IncludeError(msg);
        }
    }


    var JsEngine = function() {
        function callSite(depth) {
            var stack = depth === undefined
                    ? Qt.qst.stack()
                    : Qt.qst.stack(depth++);
            return stack;
        }

        function currentFile() {
            var cs = callSite(3);
            return cs.last().fileName;
        }

        function currentPath() {
            var cs = callSite(3);
            var filePath = cs.last().fileName;
            var fileStart = filePath.lastIndexOf('/');
            return filePath.substring(fileStart + 1);
        }

        function currentLine() {
            var cs = callSite(3);
            return cs.last().lineNumber;
        }

        return {
            currentPath: currentPath,
            currentFile: currentFile,
            currentLine: currentLine,
            callSite: callSite
        }
    }();

    return {
        require: require,
        include_check: include_check,
        Core: Core,
        Error: JsError,
        discardCallSites: discardCallSites,
        Engine: JsEngine,
        _debug: _debug,
    }
}();


function assert(condition, msg) {
    if (!condition) {
        throw new JsError(msg);
    }
}
