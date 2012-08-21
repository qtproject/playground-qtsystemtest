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

TestCollection {
    id: root
    name: 'Javascript Class api test'
    description: name

    function _() {
        // dummy
        // used instead of property var in Qt5
    }

    function setup() {
        var Class = core().Class;
        var Shape  = Class.create(function(name) {
             return {
                 name: function(){ return name; },

                 setName: function setName(value) { name = value; }
             };
        });
        _.Class = Class;
        _.Shape = Shape;
    }

    Test {
        name: 'construct objects'; description: name
        function run() {
            var Shape = _.Shape;

            var x = null;
            expect(x).toBe(null);

            x = new Shape('x');
            expect(x).not.toBe(null);
            expect(x).instanceOf(Shape);
        }
    }

    Test {
        name: 'invoke methods'; description: name
        function run() {
            var x = new _.Shape('x');
            expect(x.name()).toBe('x');
            x.setName('y');
            expect(x.name()).toBe('y');
        }

    }

    Test {
        name: 'extend objects'; description: name
        function run() {
            var Circle = _.Shape.extend(function(r) {
                this._super('Circle');
                return {
                  radius: function(){ return r; }
                };
            });

            var c = new Circle(10);
            expect(c).instanceOf(_.Shape);
            expect(c).instanceOf(Circle);
            expect(c.name()).toBe('Circle');
            expect(c.radius()).toBe(10);
        }
    }

    Test {
        name: 'private calls public'; description: name

        function setup() {
            var Class = _.Class;
            var Base = Class.create(function() {

                function bar() {
                    return this.overrideMe();
                }
                function foo() {
                    return bar.call(this);
                }
                return {
                    overrideMe: function() { return 'Base'},
                    foo: foo
                };
            });
            _.base = new Base();
        }
        function run() {
            expect(_.base.foo()).toBe('Base');
        }
    }
}
