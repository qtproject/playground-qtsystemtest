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
import QtSystemTest 1.0

Test {
    id: root
    name: 'Default application handler'; description: name

    function _() {}
    property string defaultHandler: 'Application'

    function setup() {
        var factory = core().ApplicationRegistry;
        expect(factory).not.toBe(undefined);
        _.factory = factory;
    }

    function run() {
        var
            key = 'foobar',
            factory = _.factory,
            file,
            component,
            appObject,
            className
        ;

        file = factory.fileForKey(key);
        expect(file.endsWith('/'+ defaultHandler + '.qml')).toBe(true);

        component = factory.applicationForKey(key);
        expect(component).not.toBe(undefined);
        expect(component).not.toBe(null);


        appObject = component.createObject(root);
        expect(appObject).not.toBe(undefined);
        expect(appObject).not.toBe(null);

        className = appObject.toString().split('_').first();
        expect(className).toBe(defaultHandler);
    }
}
