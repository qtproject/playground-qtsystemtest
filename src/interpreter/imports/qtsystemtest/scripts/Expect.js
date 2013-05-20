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

Js.include_check(Expectation === undefined, 'Expect.js already included');
Js.require('Class.js');
Js.require('Exception.js');

var Expectation = Class.create(function(actual, value){
    var ok = value === undefined ? true : value;

    function _not_() {
        return ok === true? ' ' : ' not ';
    }

    function toBeBetween(low, high){
       if ( ( low < actual && actual < high) !== ok) {
           var msg = 'Expected actual value to be' + _not_() + 'between low and high value.';
           var formatted = 'Expected (' +  actual +  ') to be' + _not_()
                   + 'between (' + low + ', ' + high + ')';
           var e =  new Qtt.Fail(msg, formatted, 2);
           e.meta = {
               actual: actual,
               low: low,
               high: high
           };
           throw e;
        }
    }

    function toBe(expectation){
       if ( (actual === expectation) !== ok) {
           var msg = 'Expected actual value to be' + _not_() + 'expected value.';
           var e =  new Qtt.Fail(msg, 2);
           e.meta = {
               actual: actual,
               expected: expectation
           };
           e.formatted = 'Expected (' +  actual +  ') to be' + _not_()
                   + '(' + expectation +  ')';
           throw e;
       }
    }

    function toThrow(expection){
    }

    function instanceOf(expectation){
        if( (actual instanceof expectation) !== ok) {
            var msg = 'Expected actual value to be' + _not_() + 'an instance of expected type.';
            var e =  new Qtt.Fail(msg, 2);
            e.meta = {
                actual: actual,
                expected: expectation.name
            };
            e.formated = 'Expected (' +  actual +  ') to be' + _not_()
                    + 'instance of (' +  expectation.name +  ')';
            throw e;
        }
    }


    return {
        toBe: toBe,
        toBeBetween: toBeBetween,
        instanceOf: instanceOf
    }
});


function expect(actual) {
    var x = new Expectation(actual);
    x.not =  new Expectation(actual, false)
    return x;
}
