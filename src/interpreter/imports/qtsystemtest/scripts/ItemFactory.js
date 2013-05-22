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

// create a factory and make that singleton
var ItemFactory = function() {
    /// FIXME: itemshandlers can be resitered or unregistered at run time
    /// so, the keyForObject.cache would get invalidated
    var Factory = ComponentCacher.extend(function() {
        this._super();
        this.setFallbackToDefault(false);
        var _handlerForItemCache = {};

        function keyForObject(obj, application) {
            var key = obj._type_;

            function handlerExists(k) {
                if ( this.fileForKey(k) !== null ) {
                    handlerExists.key = k;
                    return true;
                }
                return false;
            }

            // perfect match
            if ( handlerExists.call(this, key) ) {
                return key;
            }

            if ( key in _handlerForItemCache) {
                return _handlerForItemCache[key];
            }

            var tree = application.__inheritanceTree(obj);
            tree.shift();
            var ret = tree.some(handlerExists, this)
                            ? handlerExists.key
                            : null;
            _handlerForItemCache[key] = ret;
            return ret;
        }

        function itemForKey(obj, props) {
            if ( !('_type_' in obj ) ) {
                throw new Qtt.Fail('Invalid object passed to handlerForObject: _type_ does not exist ', 2);
            }

            var key = keyForObject.call(this, obj, props.application);
            if (key === null) {
                throw new Qtt.Fail('Error finding a Item handler for: ' + obj._type_ , 2);
            }
            var component = this.componentForKey(key);
            if (component === null) {
                throw new Qtt.Fail('Error creating an Item handler for: ' + obj._type_ , 2);
            }
            return component;
        }

        function register(k, path) {
            path = absolutePathOf(path);
            this._super(k, path);
            _handlerForItemCache = {};
        }

        function unregister(key) {
            this._super(key);
            _handlerForItemCache = {};
        }

        return  {
            handlerForObject: itemForKey,
            register: register,
            unregister: unregister
        };
    });
    var singleton = new Factory();
    singleton.register('QObject', '../qml/itemhandlers/QstObject.qml');
    singleton.register('QWidget', '../qml/itemhandlers/QstWidget.qml');
    return singleton;
}();
