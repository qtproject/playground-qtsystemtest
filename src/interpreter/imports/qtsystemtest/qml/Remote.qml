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

import QtQuick 1.1
import '../scripts/QstCore.js' as Core
import 'ApplicationObjectsCache.js' as Cache

QtObject {
    id: remote
    property string host
    property int port: 5656
    property int applicationStartTimeout: 5000
    property int appStartRefreshInterval: 250

    property bool __needToReconnect: true
    property bool __debug_showMessageReceived: false

    onHostChanged: { __needToReconnect = true; }
    onPortChanged: { __needToReconnect = true; }


    function runProcess(params) {
        return __sendQuery(arguments.callee.name, { cmd: params } );
    }

    function startApplication(app, args) {
        args = args || []
        args.push("-style", "qtuitestd");
        var currentAppId = __currentAppId();
        var output = runProcess({
                    program: app,
                    arguments: args,
                    detached: true
        });

        var timeout = 0;
        while( __currentAppId() === currentAppId
                &&  timeout < applicationStartTimeout) {
            sleep(appStartRefreshInterval);
            timeout += appStartRefreshInterval;
        }

        var newAppId =  __currentAppId();
        if (currentAppId === newAppId) {
            throw new Core.Qtt.Fail('Application failed to start', 1);
        }
        Cache.host(host).store(newAppId, {
                        executable: app,
                        pid: output.pid,
                    });
        return applicationForId(newAppId)
    }



    function currentApplication() {
        var appId = __currentAppId();

        if (appId === '') {
            throw new Core.Qtt.Fail('No Current application', 1);
        }

        return applicationForId(appId);
    }

    function applicationForId(appId) {
        // there should be data in the cache already
        var cache = Cache.host(host);
        var appData = cache.data(appId);
        if (appData === null) {
            throw new Core.Qtt.Fail('Application for Id not found', 1);
        }

        if (appData.application !== undefined) {
            return appData.application;
        }

        var ApplicationRegistry = Core.ApplicationRegistry
        var component = ApplicationRegistry.applicationForKey(appData.executable);
        var app = component.createObject(__runner.currentTestItem, {
                            remote: remote,
                            __id: appId,
                            pid: appData.pid,
                   });
        if (app === null) {
            throw new Core.Qtt.Error('Failed to create an application object for : ' + appId);
        }
        cache.store(appId, {application: app});
        return app;
    }

    function applications() {
    }

    function __connection() {
        if (__needToReconnect == false) {
            return __connection.object;
        }
        __connection.object = ConnectionManager.connectionForHost(host, port);
        __connection.object.messageReceived.connect(__messageReceived);
        var ok = __connection.object.connectToRemote();
        if ( !ok ) {
            var e = new Core.Qtt.Fail('Error: Connection to : \
                         host ' + host + 'port :' + port + ' failed', 2);
            throw e;
        }
        __needToReconnect = false;
        return __connection.object;
    }

    function __sendQuery(event, params, widgetSpecifier) {
        return __connection().sendQuery(event, params, widgetSpecifier);
    }

    function __sendPositionQuery(event, params, position) {
        return __connection().sendPositionQuery(event, params, position);
    }

    function __currentAppId() {
        return __sendQuery('currentApplication').currentapplication;
    }

    function __messageReceived(event, context, data) {
        if ( __debug_showMessageReceived ) {
            console.log("Message received: ",
                        event, " context(" + context + ")",
                        'data: ', Core.stringify(data));
        }
    }
}
