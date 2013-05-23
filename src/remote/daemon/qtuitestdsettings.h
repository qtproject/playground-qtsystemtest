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


#ifndef QTUITESTDSETTINGS_H
#define QTUITESTDSETTINGS_H

#include <QString>
#include <QVariantMap>

class QtUiTestDSettings
{

public:
    QtUiTestDSettings();
    ~QtUiTestDSettings();

    inline bool debug() const { return m_debug; }
    inline int port() const { return m_port; }
    inline int portMax() const { return m_portMax; }
    inline int appPortMin() const { return m_appPortMin; }
    inline int appPortMax() const { return m_appPortMax; }
    inline int prestartPort() const { return m_prestartPort; }
    inline QString prestartApp() const { return m_prestartApp; }
    inline QString homeApp() const { return m_homeApp; }
    inline QVariantMap deviceConfig() const { return m_deviceConfig; }

    static QtUiTestDSettings* instance();

private:
    bool m_debug;
    int m_port;
    int m_portMax;
    int m_appPortMin;
    int m_appPortMax;
    int m_prestartPort;
    QString m_prestartApp;
    QString m_homeApp;
    QVariantMap m_deviceConfig;
};

#endif
