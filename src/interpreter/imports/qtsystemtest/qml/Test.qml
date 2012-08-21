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
import "../scripts/QstCore.js" as Core

TestItem {
    id: root

    property string __status: Core.Qtt.Status.Untested ///'' // passed, failed, skipped
    property  bool __isTest: true
    property variant __mustHaves: ['name', 'description', 'run']

    function __validate() {
        var missingMustHave;
        var allMustHaves = __mustHaves.every(function(k) {
            missingMustHave =k;
            return (k in root) && root[k] !== '';
        });

        if (!allMustHaves) {
            return __error('Test Item: ', root.name, ' does not define property or function',
                           '\'', missingMustHave, '\' the must haves are: ', __mustHaves);
        }

        return __ok();
    }

}

