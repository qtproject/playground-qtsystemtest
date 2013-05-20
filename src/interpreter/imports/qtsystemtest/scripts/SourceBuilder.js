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

Js.include_check(SourceBuilder === undefined, 'SourceBuilder.js already included');

var SourceBuilder = function() {
    var DebugEnabled = false;
    function debug(enabled){
        if (enabled && DebugEnabled) {
            console.log( msg(Array.prototype.slice.call(arguments)) );
        }
    }

    function msg() {
        var str = '';
        for (var i =0; i<arguments.length; i++) {
            var arg = arguments[i];
            str += arg;
        }
        return str;
    }

    // converters
    function arrayToString(a, level) {
        if (a.length === 0 ) {
            return '[]';
        }

        var str = '[';
        var quote;
        for (var i =0; i < a.length; ++i) {
            quote =  typeOf(a[i]) === 'String' ? "'" : '';
            str += quote + stringify(a[i]) + quote;
            str +=  i < (a.length -1) ? ', ' : ']'
        }
        return str;
    }

    function objectToString(obj, level) {
        debug(DebugEnabled && msg(arguments.callee.name));

        // if toString for the object is overridden then, use that version
        if ( obj.toString !== Object.prototype.toString) {
            return obj.toString();
        }

        var indentation = indentText.repeat(level);
        var propertyCount = Object.keys(obj).length;
        var terminator = propertyCount >  1 ? '\n': ''
        var propertyIndentation =  propertyCount > 1 ? (indentation + indentText) : ''
        var objectType = typeOf(obj)

        function stripObject(o) {
            return o === 'Object' ? '' : o;
        }
        // don't print 'Object', if the type is Object
        var str = stripObject(objectType) + '{' + terminator;
        var contents = '';
        for (var prop in obj) {
            // add a quote if the value is of type string
            var quote =  typeof(obj[prop]) == 'string' ? "'" : '';

            contents += propertyIndentation + prop + ': '
                      + quote + stringify(obj[prop], level+1) + quote
                      + ',' + terminator;
        }
        str += contents.slice(0, -(1 + terminator.length)); // remove the , at the end of contents
        str += terminator + indentation + '}' + terminator;
        return str;
    }

    function typeOf(o) {
        debug(DebugEnabled && msg(arguments.callee.name));
        if ( o === undefined ) return 'undefined';
        if ( o === null ) return 'null';

        // we have a valid object
        var ctor = o.constructor.name;
        return ctor.length === 0 ? 'Object'  // no Object for a generic object
               : ctor;
    }

    function sameObject(x){ return x; }
    function toString(x){ return x.toString(); }
    function qtuitestWidget(x){ return 'QtUiTestWidget( ' + x.toString() + ' )'; }
    function qtuitestObject(x){ return 'QtUiTestObject( ' + x.toString() + ' )'; }

    // converts obj to string
    function stringify(obj, level) {
        debug(DebugEnabled && msg(arguments.callee.name));
        debug(DebugEnabled && msg('Finding type of :', obj) );
        level = level || 0;

        var typeConverters = {
            'null': typeOf,
            'undefined': typeOf,
            'Number': toString,
            'Boolean': toString,
            'Function': toString,
            'String': sameObject,
            'Array': arrayToString,
            'QtUiTestWidget': qtuitestWidget,
            'QtUiTestObject': qtuitestObject,
            'Object': objectToString
        };

        var type = typeOf(obj);
        if ( type in typeConverters) {
            debug(DebugEnabled && msg('Found converter for type: '  + type
                                      +  ', using converter :' + typeConverters[type].name));
            return typeConverters[type](obj, level);
        }
        debug(DebugEnabled && msg('No converters Found for type: ' + type) );
        // if no converters are registered, treat the type as an object and print it
        return objectToString(obj, level);
    }

    var indentText =  '    ';// .repeat(4); // 4 spaces for each level
    return {
        sourceFor: function sourceFor(obj) {
            return stringify(obj);
        }
    };

}// SourceBuilder

function stringify() {
    stringify.sb = stringify.sb || new SourceBuilder();
    var str = '';
    for (var i =0; i< arguments.length; ++i) {
        str += stringify.sb.sourceFor(arguments[i]);
    }
    return str;
}
