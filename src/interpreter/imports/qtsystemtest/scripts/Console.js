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

Js.include_check(Topic === undefined, 'Console.js already included');
Js.require('Class.js');

var Topic = Class.create(function(name, parent) {
    parent =  parent || null;
    var _subtopic = {
        // name: Topic
    };
    var loglevel = {
        debug: {enabled: false, activate: false },
        warn: {enabled: false, activate: false },
    }

    var DebugEnabled = false;
    function stringify() {
        stringify.sb = stringify.sb || new SourceBuilder();
        var sb = stringify.sb;
        var str = '';
        for (var i =0; i<arguments.length; ++i) {
            str += sb.sourceFor(arguments[i]);
        }
        return str;
    }
    function _debug() {
        if (DebugEnabled) {
            print('Console[Debug]: ', stringify.apply(null, arguments));
        }
    }

    function path() {
        var parents = [name];
        for (var p = parent; p !== null; p = p.parent) {
             _debug('....... type: ', typeof(p), ' ctor:', p.constructor.name );
             parents.unshift(p.name);
        }
        return parents;
    }

    function debug(m) {
        if ( loglevel.debug.activate) {
            return;
        }

        if (arguments.length === 0) {
            return;
        }
        _debug('..................', name);

        function log() {
            var parents = path();
            var location = Js.Engine.callSite(4).last();
            var filePath = location.fileName;
            var file = filePath.right(filePath.length - filePath.lastIndexOf('/') -1);
            console.log('     ', Date.timestamp(), ', '                             // timestamp
                    + file + '(' + location.lineNumber + ') :',                // file (linenumber)
                    parents.join('.'), ': ' + stringify.apply(null, arguments));    // topic.topic.... : msg
        }
//        log.apply(null, arguments);
    }

    function topic(t) {
        _debug('Already existing ones: ', Object.keys(_subtopic) );
        if ( t in  _subtopic ){
            return _subtopic[t]
        }
        _debug('Creating a topic: ', name, '.' , t);
        var sub = new Topic(t, this);
        _subtopic[t] = sub;
        _debug(Object.keys(_subtopic));
        return sub;
    }

    function enable() {

    }

    return {
        debug: debug,
        topic: topic,
        name: name,
        parent: parent,
        enable: enable,
    };
});


var Console = new Topic('Console');
