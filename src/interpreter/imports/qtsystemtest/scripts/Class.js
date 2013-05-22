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

Js.include_check(Class.isFn === undefined, 'Class.js already included');

function Class(){ };

Class.DebugEnabled = false;
//Class.DebugEnabled = true;
Class.debug = function debug() {
    if (Class.DebugEnabled) {
        var str = '';
        for (var i=0; i<arguments.length; i++) {
            str += arguments[i];
        }
        console.log(str);
    }
}

Class.isFn =  function(fn) { return typeof fn === 'function'; };
Class.isBothFn =  function (fn, superFn) {
    var debug = Class.debug;
    debug('\t    is fn ', (fn ? fn.name: undefined), ', superFn: ', (superFn ? superFn.name : undefined) , ' is a function');
    return Class.isFn(fn) && Class.isFn(superFn)
};

Class.create = function(superClass, constructor) {
    // if 2 args are passed, then SuperClass is specified and else it is of the
    // from X.extend(definition-of-y)
    // SuperClass need not be specified in that case, SubClass definition will be
    // in the SuperClass arg
    var SuperClass, SubClass,
        debug = Class.debug;
    if( arguments.length == 1) {
        SuperClass = this;
        SubClass = superClass; // has the definition

    }else {
        SubClass = constructor;
        SuperClass =  superClass;
    }
    debug('Superclass is ', SuperClass.name);

    // NOTE constructor.name for the object created using this is empty
    var definition = function () {
        // about to call the constructor of this object, so the super classes are already
        // constructor and the prototype of this is already set.
        var myProto = this;
        debug('My Prototype already has : ', Object.keys(myProto));
        // define _super before calling the construtor
        this._super = SuperClass; // _super from constructor should work

        // constructor returns the exported public members
        var publicMembers = SubClass.apply(this, arguments); // returns the exported members
        debug('Exported members are: ', Object.keys(publicMembers));

        for (var name in publicMembers) {
            debug('\t Testing if ', name, ' should be overridden');
            (function(member, superMember) {
                // overide the definition in my proto if required
                if ( Class.isFn(member) ) {
                    var s = Class.isFn(superMember) ? superMember : undefined;
                    myProto[name] = function() {
                        var tmp = this._super;
                        this._super = s;
                        var ret = member.apply(this, arguments);
                        this._super = tmp;
                        return ret;
                    }
                } else {
                    myProto[name] = member;
                }

            })(publicMembers[name], myProto[name]);
        }
    }; // definition
    definition.prototype = new SuperClass; // establish prototype chain
    definition.prototype.constructor = definition; // and the ctor
    definition.extend = SuperClass.extend || Class.create
    return definition;

}; // Class.create

