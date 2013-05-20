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

.pragma library

/*!
  Returns a function that suppresses throwing the expection when the
  function \a being wrapped either fails or throws an exception.

  The exception generated is stored in an exception property.
  The failed property indicates if the function threw an exception.

  Usage:
  \code
    var itemExists = findUniqueItem.nothrow();
    var item = itemExists({QMLTYPE: 'QQuickItem'});

    if (!itemExists.failed) {
        print(item.property('x'));
    } else {
        // item is undefined
        fail(itemExists.exception.message);
    }
  \endcode
*/
Function.prototype.nothrow = function nothrow() {
   var fn = this;
   function nothrow() {
        try {
            var result = fn.apply(this, arguments);
            arguments.callee.failed = false;
            return result;
        } catch(e) {
            arguments.callee.exception = e;
            arguments.callee.failed = true;
        }
        return undefined;
    }

    // reduce nothrow().nothrow() to a single nothrow
    return fn.toString() === nothrow.toString() ? fn : nothrow;
}

/*!
 Returns a function that retries the \a fn after every \a interval milliseconds
 until it doesn't result in a 'fail' or times out after \a timeout milliseconds.

 Usage:
 \code
    var item = findUniqueItem.retry({timeout: 1000, interval: 250});
    var obj = item({QMLTYPE: 'Button', QMLID: 'aButton'})
 \endcode

  is equal to writing

 \code
    var obj;
    var excp;
    var timeout = 1000;
    var poll = 250;
    try {
        expect( function() {
            try {
                obj = findUniqueItem({QMLTYPE: 'Button', QMLID: 'aButton'})
                return true;
            }catch(e) {
                excp = e;
            }
            return false;
        }, timeout, Math.floor(timeout/poll));
    } catch(e) {
        fail(excp.message, ' after ', timeout, 'ms' );
    }
 \endcode

*/
//Function.prototype.retry = function(config) {
//    if (!config){
//        throw new Error('Missing args: no timeout and interval specified. \n'
//                        + 'Usage retry({timeout: <timeout>, interval: <interval> })');
//    }

//    if (typeof(config.timeout)  !== 'number') {
//        throw new TypeError('timeout: not a number \n'
//                    + 'Usage retry({timeout: <timeout>, interval: <interval> })');
//    }
//    if (typeof(config.interval)  !== 'number') {
//        throw new TypeError('interval: not a number \n'
//                    + 'Usage retry({timeout: <timeout>, interval: <interval> })');
//    }

//    var method = this.nothrow();

//    function retry() {
//        // store the original method
//        retry.original = this;
//        var result;
//        var args = Private.Utils.makeArray(arguments) ;
//        var passes = Private.Utils.poll(function autoPoll() {
//           result = method.apply(this, args);
//           return method.failed === false;
//        }, config.timeout, config.interval);

//        if (!passes) {
//            fail(method.exception.message, ' after ', config.timeout, ' ms');
//        }
//        return result;
//    }

//    // error on retry().retry();
//    if (this.toString() === retry.toString()) {
//        var originalMethod = this.original.name === '' ? '[anonymous]'
//                                : this.original.name;
//        return new Error('Cannot create a retry version of already retry version of method: '
//                         + originalMethod);
//    }
//    return retry;
//}

/*!
 Partially intialize first few arguments of a function

 \code
     function sum(a, b) { return a+b;};
     var add5 = sum.curry(5);
     var x = add5(10); // x = 15 now
 \endcode

*/
Function.prototype.bind = function(scope) {
    var fn = this;
    var toArray =  Array.prototype.slice;
    var args = toArray.call(arguments);
    var scope = args.shift();
    return function() {
        return fn.apply(scope, args.concat(toArray.call(arguments)));
    };
};

Function.prototype.curry = function() {
    var fn = this;
    var toArray =  Array.prototype.slice;
    var args = toArray.call(arguments);
    return function() {
        return fn.apply(this, args.concat( toArray.call(arguments)));
    };
};


/*!
 Partially intialize arguments of a function, using undefined as placeholders for
 arguments to be filled in later.

 \code
     function sum(a, b) { return a+b;};
     var add5 = sum.partial(undefined, 5);

     var x = add5(10); // x = 15 now
 \endcode

*/
Function.prototype.partial = function(){
    var fn = this;
    var toArray =  Array.prototype.slice;
    var partialArgs = toArray.call(arguments);
    return function(){
        var args = partialArgs.slice();        // copy partial
        var arg = 0;
        for ( var i = 0; i < args.length && arg < arguments.length; i++ ) {
            if ( args[i] === undefined ) {
                args[i] = arguments[arg++];
            }
        }
        return fn.apply(this, args);
    };
}

var FunctionHelper = function() {
    /*!
      Validate number of the arguments passed to a function \a fn, throws an exception
      if the number of args passed to the \a fn is less than the minArgsRequired.

      if no minimumArgs is passed, uses the number of named arguments in \a fn

      Usage:
      \code
        var findItems = FunctionHelper.validateArgs(function findItems( filter, fetch, mode) {
        }, 1); // ensures 1 arg is passed when findItems
      \endcode
    */
    function validateArgs(fn, minArgsRequired) {
        if( typeof(fn) !== 'function' || fn.name === '' ) {
            throw new Error('The first argument to validateArgs should be a named function');
        }
        minArgsRequired = minArgsRequired || fn.length;
        return function() {
            if (arguments.length < minArgsRequired) {
                fail(fn.name, ' expects atleast ', minArgsRequired,
                     ' arguments, whereas only ', arguments.length, ' passed');
            }
            return fn.apply(this, arguments)
        };
    }

    /*!
      Deprecates a function with the name \a fnName and replaced by \a replacedBy. Would print of the
      msg if one is passed or generates one.

      Usage:
      \code
        var findWidget = FunctionHelper.deprecate('findWidget', findItems);
         or
        var findWidget = FunctionHelper.deprecate('findWidget', findItems,
                            'Usage of findWidget is deprecated, use findItems instead')
        var findWidget = FunctionHelper.deprecate('findWidget',
                            {replacedBy: findItems, name: 'findItems'});
      \endcode
    */
    function deprecate(fnName, replacedBy, msg, config) {
        msg = msg || 'Usage of deprecated function: ' + fnName + ', use '  + replacedBy.name + ' instead.';
        config = config || {
            FailOnInvoke : false
        };
        return function() {
            if ( config.FailOnInvoke ) {
                throw new Error('Error: ' + msg);
            }
            warn(msg);
            // use the function if replacedBy is a Function else if it an object, then use the replacedBy property
            var fn = replacedBy instanceof Function ? replacedBy : replacedBy.replacedBy;
            return fn.apply(this, arguments);
        };
    }


    /*!
      Helper function that wraps an object of type anything other than function
      inside an anonymous function, invoking which returns the object;

      Usage:
      \code
        var x = 10;
        x = FunctionHelper.toFunction(x);
        var y = x(); // y is 10;
      \endcode
    */
    function toFunction(x) {
        return typeof(x) === 'function' ? x : function(){ return x;};
    }

    function isFunction(x) {
        return typeof(x) === 'function';
    }
    return {
        validateArgs: validateArgs,
        deprecate: deprecate,
        toFunction: toFunction,
        isFunction: isFunction
    };
}
