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


#ifndef QTESTSLAVE_H
#define QTESTSLAVE_H

#include "qtestwidgets.h"
#include "qtuitestmessagehandler.h"

#include <QtSystemTest/QstProtocol>

class QTestSlavePrivate;

class QTSLAVE_EXPORT QTestSlave : public QstProtocol
{
    Q_OBJECT
public:
    QTestSlave();
    virtual ~QTestSlave();

    virtual void processMessage(QstMessage*);
    virtual QstMessage constructReplyToMessage(const QstMessage&);

    static QVariant getObjectProperty(QtUiTest::Object *w, const QString &property);
    static QVariant updateVariant(const QVariant &value);
    static QVariantMap objectToMap(const QObject *obj, const QStringList& = QStringList());

public slots:
    virtual void onConnected();

protected:
    void registerHandler(QtUiTest::MessageHandler *handler);
    void refreshPlugins();

private:
    QTestSlavePrivate *d;
    QMap<QString,QtUiTest::MessageHandler*> m_messageHandlers;
};

template <typename T>
T* findTestWidget(const QstMessage& message)
{
    using namespace QtUiTest;
    T* ret = 0;
    QString error;
    QString qp = message["widgetSpecifier"].toString();
    QObject *o;
    if (!QActiveTestWidget::instance()->findWidget( qp, o, error)) {
        setErrorString(error);
        return ret;
    }

    ret = qtuitest_cast<T*>(o);
    if (!ret) {
        setErrorString(QString("Error: %1 is not of type %2").arg(qp).arg(static_cast<T*>(0)->_q_interfaceName()));
    } else {
        setErrorString(QString());
    }

    return ret;
}

#endif
