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

TestMethod {
    id: root
    property string name:''
    property string description: ''
    property bool enabled: true

    property list<Remote> remotes

    property variant __mustHaves: ['name', 'description']
    property  bool __isTest: false

    function __ok() {
        var path = Core.Js.__validationRoot__;
        if ( path !== undefined) {
            path.pop();
            if (path.length === 0) {
                delete Core.Js.__validationRoot__;
            }
        }
        return {
            valid: true,
        };
    }

    function __error() {
        var runner = __runner,
            msg = Array.prototype.slice.call(arguments).join(' '),
            e = new Core.Qtt.Error(msg),
            loc = runner.locationOf(root),
            err;

        err = e.toString() + '\n'
                + 'Location: ' + loc.url + '(' + loc.lineNumber + ')';
        return {
            valid: false,
            error : err
        };
    }
}
