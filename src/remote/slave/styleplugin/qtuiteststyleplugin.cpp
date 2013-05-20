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


#include <QApplication>
#include <QStylePlugin>

#include <QtSystemTest/qtestslave.h>
#include <QtSystemTest/testslaveserver.h>

#include <QtSystemTest/QstDebug>

class QtUiTestStylePlugin : public QStylePlugin
{
    Q_OBJECT

public:
    QtUiTestStylePlugin();
    virtual ~QtUiTestStylePlugin();

    QStringList keys() const;
    QStyle *create(const QString &key);

protected:
    void initSlave(bool = false);

private:
    TestSlaveServer *m_testSlaveServer;
};

QtUiTestStylePlugin::QtUiTestStylePlugin() :
    m_testSlaveServer(0)
{
}

QtUiTestStylePlugin::~QtUiTestStylePlugin()
{
    QTT_TRACE_FUNCTION();
    m_testSlaveServer->stopService();
    delete m_testSlaveServer;
}

QStringList QtUiTestStylePlugin::keys() const
{
    QTT_TRACE_FUNCTION();
    return QStringList() << "QtUiTest_NoStyle" << QLatin1String("QtUiTest") << QLatin1String("QtUiTestD");
}

QStyle *QtUiTestStylePlugin::create(const QString &key)
{
    QTT_TRACE_FUNCTION();

    static bool initialised = false;

    if (key.toLower() == "qtuitest_nostyle") {
        // QApplication::style() has failed to initialise app style
        // so is trying all styles. In this case we DO NOT want the
        // QtUiTest style to be initialised unintentionally...
        initialised = true;
    }

    if (initialised) return 0;
    if (key.toLower() == QLatin1String("qtuitest")) {
        initSlave(false);
    } else if (key.toLower() == QLatin1String("qtuitestd")) {
        initSlave(true);
    }
    return 0;
}

void QtUiTestStylePlugin::initSlave(bool useDaemon)
{
    m_testSlaveServer = new TestSlaveServer();
    m_testSlaveServer->initSlave(useDaemon);
}

#include "qtuiteststyleplugin.moc"
#include <qplugin.h>
Q_EXPORT_PLUGIN2(QtUiTestStylePlugin, QtUiTestStylePlugin)
