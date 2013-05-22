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

.pragma library

/// FIXME: Qt5, Error.stack can be enabled by calling prepareStack once
Object.defineProperties(Error.prototype, {
    __addStack: {
        value: function(level) {
            if (this.stack) {
                return;
            }

            var stack = level === undefined ? Qt.qst.stack() : Qt.qst.stack(level);
            // this function (top of the stack) with  the right values from Error
            stack[0] = {
                fileName: this.fileName,
                lineNumber: this.lineNumber,
                functionName: '<unknown function>'
            };
            this.stack = stack;
        }
    }
});


Object.defineProperties(Array.prototype, {
    isEmpty: {
        value: function() { return this.length === 0; }
    },

    clear: {
        value: function() { this.length = 0; }
    },

    last: {
        value: function() { return this[this.length-1]; }
    },

    first: {
        value: function() { return this[0]; }
    },

    contains: {
        value: function(element) {
            if (this === undefined || this === null) {
                    throw new TypeError();
            }
            return this.indexOf(element) !== -1;
        }
    },

    remove: {
        value: function(from, to) {
          var rest = this.slice((to || from) + 1 || this.length);
          this.length = from < 0 ? this.length + from : from;
          return this.push.apply(this, rest);
        }
    }
});


Object.defineProperties(String.prototype, {
    startsWith: {
        value: function(prefix) { return this.indexOf(prefix) === 0; }
    },

    endsWith: {
        value: function(suffix) {
            return this.indexOf(suffix, this.length - suffix.length) !== -1; }
    },

    left: {
        value: function(len) { return this.substring(0, len); }
    },

    right: {
        value: function(len) { return this.substring(this.length- len); }
    },

    repeat: {
        value: function( num ) { return new Array( num + 1 ).join(this); }
    },
    trim: {
        value: function() { return this.replace(/^\s+|\s+$/g, ""); }
    }
});


Date.timestamp = function() {
    function pad(n, places) {
        var str = ''+n;
        if ( str.length >= places) {
            return str;
        }
        return '0'.repeat(places - str.length) + str;
    }
    var now = new Date();
    var timestamp =  pad(now.getHours(), 2) + ':'
            + pad( now.getMinutes(), 2) + ':'
            + pad( now.getSeconds(), 2) + ' - '
            + pad( now.getMilliseconds(), 3);
    return timestamp;
}
