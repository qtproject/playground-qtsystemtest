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

var create = function(Core){
// import to this namespace
var
    InternalError = 1<<6,
    Console = Core.Console,
    Class = Core.Class,
    Js = Core.Js,
    Qtt = Core.Qtt,
    runner,
    Topic = Console.topic('Runner')
;

function relevantStack(e){
    relevantStack.currentFile = relevantStack.currentFile
                                    || Js.Engine.currentFile();

    Topic.debug('Runner: file: ', relevantStack.currentFile);
    var filtered = [];
    e.stack.some(function(callSite) {
        var hasReachedRunner =  callSite.fileName === relevantStack.currentFile;
        if (!hasReachedRunner) {
            filtered.push(callSite);
        }
        return  hasReachedRunner;
    });
    return (e instanceof Qtt.Error)
            ? filtered
            : filtered.length > 0 ? filtered
            : e.stack;
}

var AbstractTestRunner = Class.create(function(item) {
    var Topic = Console.topic('Runner').topic('AbstractTestRunner');

    function validate(fnName, config){
        config = config || { }
        config.mustExist = config.mustExist === undefined ? true : config.mustExist;

        if ( !(fnName in item) ) {
            if (config.mustExist === true) {
                throw new ReferenceError( 'Missing mandatory property or function "'
                    + fnName + '" in test item (Name:' +  item.name + ')')
            }
            Topic.debug('Name: ', fnName, ' is missing, but ok to  ignore' );
            return {ok: true, exists : false};
        }

        var m = item[fnName];
        if (typeof (m) !== 'function') {
            throw new TypeError(fnName + ' not a function');
        }
        return {ok: true, exists : true};
    }

    function exec() {
        var  Topic = Console.topic('Runner').topic('exec');
        Topic.debug( 'Running .............', item);

        // skip disabled items
        var result = true;
        if (item.enabled === false) {
            Topic.debug(item, 'is disabled ');
            return result;
        }

        this.start();
        runner.currentTestItem = item;
        try {
            var ok = invokeSetup.call(this);
            if (ok) {
                this.run();
            }
        } catch (e) {
            /// FIXME: Qt5, remove this as e.stack *can be* enabled by default
            e.__addStack(1);
            this.setStatus(Qtt.Status.Failed);
            console.log("Internal error: ", e.status || 'JsError', e.message);
            ok = false;
            e.stack.forEach( function(s) {
                   console.log(s.fileName, ': ', s.lineNumber, ' : ', s.functionName); });

            var error = {
                message: e.message,
                stack: relevantStack(e),
                formatted: e.formatted,
                meta: e.meta
            };
            runner.logEvent(e.status || 'JsError', error);
            throw e;
        } finally {
            invokeCleanup.call(this);
            this.end();
        }
    }


    function invokeSetup() {
        var   Topic = Console.topic('AbstractTestRunner').topic('invokeSetup');

        Topic.debug( 'validating setup');
        var setup = 'setup';
        var result = validate(setup, {mustExist: false});
        if ( result.ok && result.exists === false ) {
            return true;
        }

        setup = item.setup;
        var ok = true;
        try {
            runner.methodStart(setup.name);
            runner.currentMethod = setup.name;
            setup();
            runner.methodEnd(setup.name);
        } catch (e) {
            /// FIXME: Qt5, remove this as e.stack *can be* enabled by default
            e.__addStack(1);
            Topic.debug('Exception caught: ', e);
            ok = false;
            var status = (e instanceof Qtt.Error)
                            ? Qtt.Status.Untested
                            : Qtt.Status.Failed;
            var error = {
                message: e.message,
                stack: relevantStack(e),
                formatted: e.formatted,
                meta: e.meta
            };
            runner.logEvent(e.status || 'JsError', error);
            runner.methodEnd(setup.name);
            this.setStatus(status);
        }
        return ok;
    }

    // don't throw any errors, just report them and end the cleanup
    function invokeCleanup() {
        var Topic = Console.topic('TestRunner').topic('runCleanup');

        var cleanup = 'cleanup';
        var result = validate(cleanup, {mustExist: false});
        if ( result.ok && result.exists === false ) {
            return;
        }

        cleanup = item.cleanup;
        runner.methodStart(cleanup.name);
        runner.currentMethod = cleanup.name;

        try {
            cleanup();
        } catch (e) {
            /// FIXME: Qt5, remove this as e.stack *can be* enabled by default
            e.__addStack(1);
            Topic.debug('Exception in cleanup: ', e);

            var error = {
                message: e.message,
                stack: e.stack,
                formatted: e.formatted,
                meta: e.meta
            };

            runner.logEvent('WARNING', error);
        } finally {
            Topic.debug('run finally - finishing method : cleanup ');
            runner.methodEnd(cleanup.name);
        }
    }

    return {
        validate: validate,
        item: function() { return item },
        exec: exec,
        start: function() { throw new Error('Must override start')},
        run: function() { throw new Error('Must override run')},
        end: function() { throw new Error('Must override end')},
        setStatus: function() { throw new Error('Must override setStatus')},
    };
});

var TestRunner = AbstractTestRunner.extend(function(item) {
    this._super(item);
    var   Topic = Console.topic('TestRunner').topic('AbstractTestRunner');
    Topic.debug(' Creating a Test runner for ', item.name);

    function start() { runner.testStart(item); }

    function end() { runner.testEnd(item); }

    function setStatus(status) {
        Topic.debug('Setting ... ', item.name ,  ' :', status)
        item.__status = status;
        /// FIXME Qt5: change report to var and runner.report[status]++
        var report = runner.report;
        report[status]++;
        runner.report = report;
    }

    function run() {
        var   Topic = Console.topic('TestRunner').topic('runRun');

        Topic.debug( 'validating run');
        var run = 'run';
        var status = Qtt.Status.Failed;
        try {
            this.validate(run);

            run = item.run;
            runner.methodStart(run.name);
            runner.currentMethod = run.name;

            Topic.debug('Going to run ... run');
            run();
            status = Qtt.Status.Passed;
        } catch (e) {
            /// FIXME: Qt5, remove this as e.stack *can be* enabled by default
            e.__addStack(1);
            status = (e instanceof Qtt.Error)
                        ? e.status
                        : Qtt.Status.Failed;
            var error = {
                message: e.message,
                stack: relevantStack(e),
                formatted: e.formatted,
                meta: e.meta
            };
            Topic.debug(status, Core.stringify(error));
            runner.logEvent(status, error);
        } finally {
            runner.methodEnd(run.name);
            this.setStatus(status);
        }
    }

    return {
        start: start,
        end: end,
        run: run,
        setStatus: setStatus
    };
});

var TestCollectionRunner = AbstractTestRunner.extend(function(item) {
    this._super(item);


    function start() { runner.testCollectionStart(item); }

    function end() { runner.testCollectionEnd(item); }


    function setStatus(status) {
        // recurse and run the subsets ...
        var subset = item.children;
        for (var i = 0; i< subset.length; ++i) {
            var testItem = subset[i];
            if ( !testItem.enabled ) {
                continue;
            }
            var tr = runnerForItem(testItem);
            tr.start();
            tr.setStatus(status);
            tr.end();
        }
    }

    function run() {
        // recurse and run the subsets ...
        var subset = item.__subset;
        for (var i = 0; i< subset.length; ++i) {
            var tr = runnerForItem(subset[i]);
            tr.exec();
        }
    }

    return {
        start: start,
        end: end,
        run: run,
        setStatus: setStatus
    };
});


function runnerForItem(item) {
    var Runner = item.__isTest ? TestRunner : TestCollectionRunner;
    return new Runner(item);
}

function exec(qmlRunner, item){
    runner = qmlRunner
    try {
        runner.aboutToStartExecution();
        var tr =  runnerForItem(item);
        tr.exec();
        runner.finishedExecution();

        var report = runner.report;
        var result  = (report.failed > 0)
                         | (report.skipped > 0) << 1
                         | (report.untested > 0) << 2
                         | (report.error > 0) << 3;
    } catch (e) {
        /// FIXME: Qt5, remove this as e.stack *can be* enabled by default
        e.__addStack(1);
        result = InternalError;
    }
    return result;
}

    return {
        exec: exec
    }

};
