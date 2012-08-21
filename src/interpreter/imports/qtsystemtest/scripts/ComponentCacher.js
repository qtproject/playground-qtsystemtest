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

Js.include_check(ComponentCacher === undefined, 'ComponentCacher.js already included');
Js.require('Exception.js');
Js.require('Class.js');

var OriginalCallerDepth = 5; // explaination below
/*
  the stack for factory.register(foo, bar) or this.setDefaultItemFile would  lead to
  1. something.register()
  2. class.call (register)
  3. this.register
  3.      absolutePathOf
  4.            inside register
  to get the file that called register, we would have to traverse back in stack 5 levels
*/

    function absolutePathOf(path) {
        if ( !Qt.qst.isRelativePath(path) ) {
            return path;
        }
        // get the filename that called applicaton.register
        var stack = Js.Engine.callSite(OriginalCallerDepth);
        var callerFilePath = stack.last().fileName;
        var lastSlash = callerFilePath.lastIndexOf('/');
        return callerFilePath.left(lastSlash + 1) + path;
    }


var ComponentCacher = Class.create(function() {
    var __registry = {
        //key: qmlFile
    };

    var __componentsCache = {
        //key: Component
    };
    var __defaultItemFile;
    var fallbackToDefault = true;

    function setDefaultItemFile(qmlFilePath) {
        __defaultItemFile = absolutePathOf(qmlFilePath);
    }

    function unregister(key)  {
        if (key in __registry) {
           delete __registry[key];
        }
        if (key in __componentsCache) {
           delete __componentsCache[key];
        }
    }

    function register(key, qmlFilePath)  {
        __registry[key] = absolutePathOf(qmlFilePath);
    }

    function fileForKey(key) {
        if (key in __registry) {
            return __registry[key];
        }
        if ( !fallbackToDefault) {
            return null;
        }

        if ( !__defaultItemFile) {
            throw new Qtt.Error( 'Error: ComponentCacher - unable to find a file for ' + key);
        }
        return __defaultItemFile;
    }

    function componentForKey(key) {
        var qmlFile = this.fileForKey(key);
        if (qmlFile === null) {
            return null;
        }

        var component = null;
        if ( ! (qmlFile in __componentsCache)) {

            component = Qt.createComponent(qmlFile)
            // HACK ... 1 means Component.Ready I want to access Component.Ready here...
            if (component.status !== 1) {
                // fix me ... Change error to accept msg and formatted msg,
                throw new Qtt.Error('Component creation failed : ' + component.errorString(), 2);
            }
            __componentsCache[qmlFile] = component;
        } else  {
            component = __componentsCache[qmlFile];
        }

        return component;
    }
    function setFallbackToDefault(value) {
        fallbackToDefault = value;
    }

    return {

        setFallbackToDefault: setFallbackToDefault,
        setDefaultItemFile: setDefaultItemFile,
        fileForKey: fileForKey,
        register: register,
        unregister: unregister,
        componentForKey: componentForKey,
        keys: function() { return  Object.keys(__registry); },
    };
});