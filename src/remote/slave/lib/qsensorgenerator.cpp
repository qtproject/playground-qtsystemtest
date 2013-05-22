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


#include "qsensorgenerator_p.h"
#include <qtuitestnamespace.h>
#include <QtSystemTest/QstDebug>
#include <QTimerEvent>
#include <QTimer>
#include <QDateTime>
#include <QMetaObject>
#include <QMetaProperty>

static void copySensorProperties(QObject *source, QObject *dest)
{
    const QMetaObject *meta = source->metaObject();
    for (int i = QSensor::staticMetaObject.propertyOffset(); i < meta->propertyCount(); i++) {
        QMetaProperty prop = meta->property(i);
        // Not these ones...
        if (prop.name() == QLatin1String("type") ||
                prop.name() == QLatin1String("sensorid") ||
                prop.name() == QLatin1String("connectedToBackend") ||
                prop.name() == QLatin1String("availableDataRates") ||
                prop.name() == QLatin1String("dataRate") ||
                prop.name() == QLatin1String("reading") ||
                prop.name() == QLatin1String("busy") ||
                prop.name() == QLatin1String("active") ||
                prop.name() == QLatin1String("outputRanges") ||
                prop.name() == QLatin1String("outputRange") ||
                prop.name() == QLatin1String("description") ||
                prop.name() == QLatin1String("error")) {
            continue;
        }
        dest->setProperty(prop.name(), prop.read(source));
    }
}

QtUiTestSensorBackend::QtUiTestSensorBackend(QSensor *sensor, const QByteArray &sensorId, const QByteArray &fallThroughId)
    : QSensorBackend(sensor), m_fallThrough(0), m_mode(PassThrough), m_timerId(0)
{
    // see if we have a fall-through sensor
    if (!fallThroughId.isEmpty()) {
        QList<QByteArray> list = sensor->sensorsForType(sensor->type());
        if (list.contains(fallThroughId)) {
            m_fallThrough = new QSensor(sensor->type(), this);
            m_fallThrough->setIdentifier(fallThroughId);

            // Connect to the fallthrough sensor
            bool connectedToFallThrough = m_fallThrough->connectToBackend();
            Q_ASSERT(connectedToFallThrough);

            // Set informative properties
            setDescription(m_fallThrough->description());
            setDataRates(m_fallThrough);
            foreach (const qoutputrange &r, m_fallThrough->outputRanges()) {
                addOutputRange(r.minimum, r.maximum, r.accuracy);
            }

            // Set any other properties from the fallthrough sensor
            copySensorProperties(m_fallThrough, sensor);

            // Handle signals from the fallthrough sensor
            connect(m_fallThrough, SIGNAL(readingChanged()), this, SLOT(forwardFallthroughReading()));
            connect(m_fallThrough, SIGNAL(sensorError(int)), sensor, SIGNAL(sensorError(int)));
            connect(m_fallThrough, SIGNAL(busyChanged()), sensor, SIGNAL(busyChanged()));
        }
    }

    // set the mode to match any existing active backends
    setMode(mode(sensorId));

    if (!m_fallThrough) {
        addDataRate(100, 100);
    }

    // add ourselves to the list of backends.
    activeBackends().append(this);
}

QtUiTestSensorBackend::~QtUiTestSensorBackend()
{
    activeBackends().removeAll(this);
}

int QtUiTestSensorBackend::interval() const
{
    int dataRate, interval;
    dataRate = sensor()->dataRate();
    if (dataRate == 0) {
        if (sensor()->availableDataRates().count())
            // Use the first available rate when -1 is chosen
            dataRate = sensor()->availableDataRates().first().first;
        else
            dataRate = 1;
    }

    interval = 1000 / dataRate;
    return interval;
}

void QtUiTestSensorBackend::start()
{
    if (m_timerId)
        return;

    switch (m_mode) {
    case PassThrough:
    case Record:
        if (m_fallThrough) {
            m_fallThrough->setDataRate(sensor()->dataRate());
            m_fallThrough->setOutputRange(sensor()->outputRange());
            // Set extra properties onto the fallthrough sensor
            copySensorProperties(sensor(), m_fallThrough);
            m_fallThrough->start();
            if (!m_fallThrough->isActive())
                sensorStopped();
        }
        m_timerId = -1;
        break;
    case Generate:
        m_timerId = startTimer(interval());
        break;
    }
}

void QtUiTestSensorBackend::stop()
{
    if (!m_timerId)
        return;

    switch (m_mode) {
    case PassThrough:
    case Record:
        if (m_fallThrough)
            m_fallThrough->stop();
        m_timerId = 0;
    case Generate:
        killTimer(m_timerId);
        m_timerId = 0;
    }
}

void QtUiTestSensorBackend::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != m_timerId)
        return;
    switch (m_mode) {
    case PassThrough:
    case Record:
        break;
    case Generate:
        if (m_testData.count() > 1)
            setCurrentTestData(m_testData.takeFirst());
        else if (!m_testData.isEmpty())
            setCurrentTestData(m_testData.first());
        break;
    }
}

void QtUiTestSensorBackend::setMode(Mode mode)
{
    if (m_mode == mode)
        return;
    // test data shouldn't survive mode changes
    // as it has diffeerent meanings in each mode
    m_testData.clear();
    // check if we need to turn timers on/off
    if (m_timerId) {
        if (m_mode == Generate) {
            killTimer(m_timerId);
            m_timerId = -1;
        } else if (mode == Generate) {
            m_timerId = startTimer(interval());
        }
    }
    m_mode = mode;
    // some sensors are not noisy, get at least one value updated
    // for new mode.
    forwardFallthroughReading();
}

void QtUiTestSensorBackend::queueTestData(const QList<QVariant> &data)
{
    m_testData = data;
}

void QtUiTestSensorBackend::forwardFallthroughReading()
{
    if (!m_fallThrough->reading())
        return;

    QVariant data;
    QList<QVariant> parts;
    int i;
    switch (m_mode) {
    case PassThrough:
    case Record:
        // copy sensor reading to reading
        parts << m_fallThrough->reading()->timestamp();
        for (i = 0; i < m_fallThrough->reading()->valueCount(); ++i)
            parts << m_fallThrough->reading()->value(i);
        data = QVariant(parts);
        setCurrentTestData(data);
        if (m_mode == Record) {
            // from above, + store the value
            m_testData.append(data);
        }
        break;
    case Generate:
        break;
    }
}

void QtUiTestSensorBackend::setMode(const QByteArray &id, Mode mode)
{
    backendModes().insert(id, mode);
    foreach (QtUiTestSensorBackend *backend, activeBackends()) {
        if (backend->identity() == id)
            backend->setMode(mode);
    }
}

QtUiTestSensorBackend::Mode QtUiTestSensorBackend::mode(const QByteArray &id)
{
    return backendModes().value(id, PassThrough);
}

void QtUiTestSensorBackend::queueTestData(const QByteArray &id, const QList<QVariant> &data)
{
    foreach (QtUiTestSensorBackend *backend, activeBackends()) {
        if (backend->identity() == id)
            backend->queueTestData(data);
    }
}

QList<QVariant> QtUiTestSensorBackend::testData(const QByteArray &id)
{
    foreach (QtUiTestSensorBackend *backend, activeBackends()) {
        if (backend->identity() == id)
            return backend->testData();
    }
    return QList<QVariant>();
}

QList<QtUiTestSensorBackend *> &QtUiTestSensorBackend::activeBackends()
{
    static QList<QtUiTestSensorBackend *> list;
    return list;
}

QMap<QByteArray, QtUiTestSensorBackend::Mode> &QtUiTestSensorBackend::backendModes()
{
    static QMap<QByteArray, Mode> map;
    return map;
}

const char * QAccelerometerGenerator::id("com.nokia.qtuitest.sensor.accelerometer");

QAccelerometerGenerator::QAccelerometerGenerator(QSensor *sensor, const QByteArray &fallThroughId)
    : QtUiTestSensorBackend(sensor, identity(), fallThroughId)
{
    m_reading.setTimestamp(0);
    m_reading.setX(0.0);
    m_reading.setY(0.0);
    m_reading.setZ(0.0);
    setReading(&m_reading);
}

QAccelerometerGenerator::~QAccelerometerGenerator()
{
}

void QAccelerometerGenerator::setCurrentTestData(const QVariant &value)
{
    QVariantList list = value.toList();
    if (list.count() == 4) {
        m_reading.setTimestamp(list.at(0).value<quint64>());
        m_reading.setX(list.at(1).toReal());
        m_reading.setY(list.at(2).toReal());
        m_reading.setZ(list.at(3).toReal());
        newReadingAvailable();
    } else if (list.count() == 3) {
        m_reading.setTimestamp(QDateTime::currentMSecsSinceEpoch());
        m_reading.setX(list.at(0).toReal());
        m_reading.setY(list.at(1).toReal());
        m_reading.setZ(list.at(2).toReal());
        newReadingAvailable();
    }
}

const char * QOrientationGenerator::id("com.nokia.qtuitest.sensor.orientation");

QOrientationGenerator::QOrientationGenerator(QSensor *sensor, const QByteArray &fallThroughId)
    : QtUiTestSensorBackend(sensor, identity(), fallThroughId)
{
    m_reading.setTimestamp(0);
    m_reading.setOrientation(QOrientationReading::Undefined);
    setReading(&m_reading);
}

QOrientationGenerator::~QOrientationGenerator()
{
}

void QOrientationGenerator::setCurrentTestData(const QVariant &value)
{
    QVariantList list = value.toList();
    if (list.count() == 2) {
        m_reading.setTimestamp(list.at(0).value<quint64>());
        m_reading.setOrientation(static_cast<QOrientationReading::Orientation>(list.at(1).toInt()));
        newReadingAvailable();
    } else if (list.count() == 1) {
        m_reading.setTimestamp(QDateTime::currentMSecsSinceEpoch());
        m_reading.setOrientation(static_cast<QOrientationReading::Orientation>(list.at(0).toInt()));
        newReadingAvailable();
    }
}
