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

import QtQuick 1.1
import '..'
import "../../scripts/QstCore.js" as Core

QtObject {
    id: me

    property Application application
    property variant __internals
    property bool __isQmlWrapper: true
    property string handles: 'QObject'

    function property(name) {
        return __sendQuery('getProperty', {property: name}).getproperty;
    }

    function setProperty(name, value) {
        return __sendQuery('setProperty', {
                    property: name,
                    value: value,
                });
    }

    function descendants(filter) {
         filter = filter || {};
         filter.descendant_of = me;
         return application.items(filter);
    }

    function ancestors(filter) {
         filter = filter || {};
         filter.ancestor_of = me;
         return application.items(filter);
    }

    function parent() {
        return application.item({parent_of: me});
    }

    function isValid() {
        try {
            var x = type();
        } catch(e) {
            return false;
        }
       return true;

    }

    function type() {
        return __sendQuery('typeOf')['typeof'];
    }

//    method: function(methodSignature) {
//        return new QtUiTestMethod(this, methodSignature);
//    },

//    equals: function(other) {
//        // FIXME other should be checked if it is a QtUiTestObject
//        // but that test fails since for _prefetched_ objects the
//        // type is set as Object and not QtUiTestWidget or QtUiTestObject
//        // try: print(other.constructor.name)
//        return other._signature_ !== undefined
//         && this._signature_ === other._signature_;
//    },

    function toString() {
       return this._signature_;
    }

    function __sendQuery(event, params) {
        params = params || {};
        return application.__sendQuery(event, params, __internals);
    }

}
