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
import "../scripts/QstCore.js" as Core

QtObject {
    id: app

    property  Remote remote
    property variant pid: null
    property variant __id

    function item(query) {
        var matches = __queryProperties(query);
        if ( matches.length !== 1 ) {
            throw new Core.Qtt.Fail('Error: ' + matches.length +
                         ' items match query, expected a single match', 1);
        }
        return __handlerForObject(matches[0]);
    }

    function items(query) {
        var matches = __queryProperties(query);
        var handlers = matches.map(function(obj) {
            return __handlerForObject(obj);
        });
        return handlers;
    }

    function activeWidgetInfo() { return __sendQuery(arguments.callee.name); }

    function applicationData() {
        return __sendQuery(arguments.callee.name).applicationdata;
    }


    function keyPress(key) {
        __sendQuery(arguments.callee.name, { key: key });
    }

    function keyRelease(key) {
        __sendQuery(arguments.callee.name, { key: key });
    }

    function keyClick(key) {
        __sendQuery(arguments.callee.name, { key: key });
    }


    function mousePress(pos, buttons) {
        __inputEvent(arguments.callee.name, pos, buttons);
    }

    function mouseRelease(pos, buttons) {
        __inputEvent(arguments.callee.name, pos, buttons);
    }

    function mouseClick(pos, buttons) {
        __inputEvent(arguments.callee.name, pos, buttons);
    }

    function mouseDoubleClick(pos, buttons) {
        __inputEvent(arguments.callee.name, pos, buttons);
    }

    function terminate() {
        remote.runProcess( {  program: "kill", arguments: [ pid ], timeout: 500 } );
    }

    function __queryProperties(query) {
        // traverse the query object replacing any QstObject with its __internals
        (function prepare(query) {
            var prop, v;
            for (prop in query) {
                v = query[prop];
                if ( typeof(v) !== 'object' ) { continue; }

                if( v.__isQmlWrapper === true ) {
                    query[prop] = v.__internals;
                } else {
                  prepare(v);
                }
            }
        })(query);

        var items =  __sendQuery("queryProperties",  {
                            searchvalues: query
                     }).queryproperties;
        return items || [];
    }

    function __handlerForObject(obj) {
        var ItemFactory = Core.ItemFactory;
        var component = ItemFactory.handlerForObject(obj, {application: app});
        return component.createObject(
                __runner.currentTestItem, {
                    application: app,
                    __internals: obj,
                }
        );

    }


    function __inheritanceTree(o) {
        return __sendQuery('inheritance',{},  o).inheritance;
    }

    function __inputEvent(event, pos, buttons) {
        buttons = buttons || Qt.LeftButton;

        if (pos.__isQmlWrapper === true) {
            return __sendQuery(event, {buttons: buttons}, pos.__internals);
        }

        return remote.__sendPositionQuery(event, {buttons: buttons}, pos);
    }


    function __sendQuery() {
        return remote.__sendQuery.apply(remote, arguments);
    }
}
