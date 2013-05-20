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


#ifndef QSENSORGENERATOR_P_H
#define QSENSORGENERATOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtUiTest API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGlobal>
#include <QtSystemTest/qstglobal.h>

#include <QSensorBackend>
#include <QAccelerometerReading>
#include <QOrientationReading>
#include <QSensorBackendFactory>
#include <QList>

class QTUITEST_EXPORT QtUiTestSensorBackend : public QSensorBackend
{
Q_OBJECT
public:
    QtUiTestSensorBackend(QSensor *sensor, const QByteArray &sensorId, const QByteArray &fallThroughId);
    ~QtUiTestSensorBackend();

    void start();
    void stop();

    enum Mode {
        PassThrough = 0,
        Generate = 1,
        Record = 2
    };

    static void setMode(const QByteArray &id, Mode mode);
    static Mode mode(const QByteArray &);
    static void queueTestData(const QByteArray &, const QList<QVariant> &);
    static QList<QVariant> testData(const QByteArray &);

    virtual QByteArray identity() const = 0;

protected:
    void timerEvent(QTimerEvent *);

    void setMode(Mode mode);
    Mode mode() const { return m_mode; }
    void queueTestData(const QList<QVariant> &);
    QList<QVariant> testData() { return m_testData; }

    virtual void setCurrentTestData(const QVariant &) = 0;

private slots:
    void forwardFallthroughReading();

private:
    int interval() const;

    QSensor *m_fallThrough;
    QList<QVariant> m_testData;
    Mode m_mode;
    int m_timerId;

    static QList<QtUiTestSensorBackend *> &activeBackends();
    static QMap<QByteArray, Mode> &backendModes();
};

class QTUITEST_EXPORT QAccelerometerGenerator : public QtUiTestSensorBackend
{
Q_OBJECT
public:
    QAccelerometerGenerator(QSensor *sensor, const QByteArray &fallThroughId = QByteArray());
    ~QAccelerometerGenerator();

    QByteArray identity() const { return id; }
    static const char *id;

protected:
    void setCurrentTestData(const QVariant &);

private:
    QAccelerometerReading m_reading;
};

class QTUITEST_EXPORT QOrientationGenerator : public QtUiTestSensorBackend
{
Q_OBJECT
public:
    QOrientationGenerator(QSensor *sensor, const QByteArray &fallThroughId = QByteArray());
    ~QOrientationGenerator();

    QByteArray identity() const { return id; }
    static const char *id;

protected:
    void setCurrentTestData(const QVariant &);

private:
    QOrientationReading m_reading;
};

class QTUITEST_EXPORT QSensorGeneratorFactory : public QSensorBackendFactory
{
public:
    QSensorGeneratorFactory()
    {}

    void registerSensors() {
        m_previousDefaultSensors.insert(QAccelerometer::type,
                                        QSensor::defaultSensorForType(QAccelerometer::type));
        m_previousDefaultSensors.insert(QOrientationSensor::type,
                                        QSensor::defaultSensorForType(QOrientationSensor::type));

        QSensorManager::registerBackend(QAccelerometer::type, QAccelerometerGenerator::id, this);
        QSensorManager::registerBackend(QOrientationSensor::type, QOrientationGenerator::id, this);

        QSensorManager::setDefaultBackend(QAccelerometer::type, QAccelerometerGenerator::id);
        QSensorManager::setDefaultBackend(QOrientationSensor::type, QOrientationGenerator::id);
    }

    QSensorBackend *createBackend(QSensor *sensor)
    {
        QByteArray wrapped = m_previousDefaultSensors.value(sensor->type(), QByteArray());
        if (sensor->identifier() == QAccelerometerGenerator::id) {
            if (wrapped == QAccelerometerGenerator::id)
                return 0;
            return new QAccelerometerGenerator(sensor, wrapped);
        }
        if (sensor->identifier() == QOrientationGenerator::id) {
            if (wrapped == QOrientationGenerator::id)
                return 0;
            return new QOrientationGenerator(sensor, wrapped);
        }
        return 0;
    }
private:
    QMap<QByteArray, QByteArray> m_previousDefaultSensors;
};

#endif // QSENSORGENERATOR_P_H

