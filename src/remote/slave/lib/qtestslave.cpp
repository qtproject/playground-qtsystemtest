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


#include <qtestslave.h>
#include <qtestwidgets.h>
#include <qtuitestnamespace.h>
#include <qtuitestconnectionmanager_p.h>

#include <QImage>
#include <QCoreApplication>
#include <QVariant>
#include <QMetaProperty>
#include <QMetaObject>
#include <QMetaType>
#include <QPoint>
#include <QLocale>
#include <QtSystemTest/QstDebug>
#include <QElapsedTimer>
#include <QtGui/QPlatformIntegration>

#include <QDir>
#include <QPluginLoader>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    #include <private/qguiapplication_p.h>
//  #include <qsensorgenerator_p.h>
    #include <qplatforminputcontext_qpa.h>
#endif

#ifdef QTUITEST_CPU_TICKS
# include <QtTest/private/cycle_p.h>
#endif

#define qLog() if(1); else QtUiTest::debug()

using namespace QtUiTest;

class QtUiTestSignalReceiver : public QObject
{
public:
    QtUiTestSignalReceiver(QTestSlavePrivate* = 0);

    bool connectSignal(QObject *sender, const char *signal);
    QVariantList argValues() const { return m_argValues; }
    int qt_metacall(QMetaObject::Call call, int methodId, void **a);
    virtual void signalReceived();

protected:
    QTestSlavePrivate *m_slave;
    QObject *m_sender;
    QString m_signal;
    bool m_received;
    QList<int> m_args;
    QVariantList m_argValues;
};

class QtUiTestSignalWaiter : public QtUiTestSignalReceiver
{
public:
    QtUiTestSignalWaiter(QTestSlavePrivate* = 0);

    bool setExpectedSignal(QObject *sender, const char *signal);
    void clear();
    bool waitForSignal( int timeout = 10000 );
    virtual void signalReceived();
    int elapsed() const { return m_elapsed; }

private:
    QEventLoop m_eventLoop;
    QTime m_time;
    int m_elapsed;
};

typedef QPair<QObject*,QString> ReceiverKey;

// Handler for test messages
class QTestSlavePrivate : public QObject
{
Q_OBJECT
public:
    QTestSlavePrivate(QTestSlave *parent)
        :   p(parent)
    {
    }

    QPoint mousePointForMessage(const QstMessage&);
    QVariantList touchPointsForMessage(const QstMessage&);

    void signalReceived(QObject *sender, const QString &signal, const QVariantList &argValues);

    void resetBenchmarks();
    qint64 cpuTicks();

    QTestSlave *p;
    QtUiTestSignalWaiter m_signalWaiter;
    QMap<ReceiverKey,QtUiTestSignalReceiver*> m_signalReceivers;

#ifdef QTUITEST_CPU_TICKS
    CycleCounterTicks startTicks;
#endif

public slots:
    QstMessage appName            (const QstMessage&);
    QstMessage isVisible          (const QstMessage&);
    QstMessage hasContents        (const QstMessage&);

    QstMessage grabImage          (const QstMessage&);
    QstMessage window             (const QstMessage&);
    QstMessage getCenter          (const QstMessage&);
    QstMessage enterText          (const QstMessage&);
    QstMessage isHigher           (const QstMessage&);

    QstMessage inputMethod        (const QstMessage&);

    QstMessage activeWidgetInfo   (const QstMessage&);
    QstMessage getObjects         (const QstMessage&);

    QstMessage applicationData    (const QstMessage&);
    QstMessage applicationWindows (const QstMessage&);
    QstMessage windowProperty     (const QstMessage&);
    QstMessage invokeMethod       (const QstMessage&);
    QstMessage setProperty        (const QstMessage&);
    QstMessage getProperty        (const QstMessage&);
    QstMessage getProperties      (const QstMessage&);
    QstMessage queryProperties    (const QstMessage&);

    QstMessage getGeometry        (const QstMessage&);

    QstMessage keyPress           (const QstMessage&);
    QstMessage keyRelease         (const QstMessage&);
    QstMessage keyClick           (const QstMessage&);

    QstMessage touchPress         (const QstMessage&);
    QstMessage touchUpdate        (const QstMessage&);
    QstMessage touchRelease       (const QstMessage&);

    QstMessage mousePress         (const QstMessage&);
    QstMessage mouseRelease       (const QstMessage&);
    QstMessage mouseClick         (const QstMessage&);
    QstMessage mouseDoubleClick   (const QstMessage&);
    QstMessage mouseMove          (const QstMessage&);

    QstMessage translate          (const QstMessage&);
    QstMessage trId               (const QstMessage&);
    QstMessage getLocale          (const QstMessage&);
    QstMessage setConfig          (const QstMessage&);
    QstMessage rescan             (const QstMessage&);
    QstMessage typeOf             (const QstMessage&);
    QstMessage inheritance        (const QstMessage&);
    QstMessage queuedMessage      (const QstMessage&);

    QstMessage getMethods         (const QstMessage&);
    QstMessage expectSignal       (const QstMessage&);
    QstMessage waitForSignal      (const QstMessage&);

    QstMessage connectSignal      (const QstMessage&);
    QstMessage disconnectSignal   (const QstMessage&);
    QstMessage cleanup            (const QstMessage&);

/*
    QstMessage setSensorMode      (const QstMessage&);
    QstMessage getSensorMode      (const QstMessage&);
    QstMessage queueSensorValues  (const QstMessage&);
    QstMessage getSensorValues    (const QstMessage&);
*/
protected slots:
    void disconnectAllSignals(QObject*);

};

#include "qtestslave.moc"

QTestSlave::QTestSlave()
    : QstProtocol()
    , d(new QTestSlavePrivate(this))
{
    QTT_TRACE_FUNCTION();
    QtUiTest::testInputOption(QtUiTest::NoOptions);
    refreshPlugins();
}

QTestSlave::~QTestSlave()
{
    QTT_TRACE_FUNCTION();
    delete d;
    disconnect();
}

void QTestSlave::onConnected()
{
    QTT_TRACE_FUNCTION();
    QstProtocol::onConnected();
    QstMessage msg("APP_NAME");
    msg["appName"] = qApp->applicationName();
    msg["appVersion"] = qApp->applicationVersion();
    msg["qtVersion"] = QT_VERSION_STR;
    postMessage( msg );
}

QtUiTestSignalReceiver::QtUiTestSignalReceiver(QTestSlavePrivate *slave)
    : m_slave(slave)
    , m_sender(0)
    , m_received(false)
{
}

bool QtUiTestSignalReceiver::connectSignal(QObject *sender, const char *signal)
{
    m_sender = sender;
    m_signal = QString(signal);

    const QMetaObject* const senderMo = sender->metaObject();
    static const int memberOffset = QObject::staticMetaObject.methodCount();

    int signalIndex = senderMo->indexOfSignal(signal);
    if (signalIndex < 0) {
        signalIndex = senderMo->indexOfSignal(QMetaObject::normalizedSignature(signal));
    }
    if (signalIndex < 0) {
        return false;
    }

    QMetaMethod method = senderMo->method(signalIndex);
    QList<QByteArray> params = method.parameterTypes();
    for (int i = 0; i < params.count(); ++i) {
        m_args << QMetaType::type(params.at(i).constData());
    }

    QtUiTestConnectionManager::instance()->connect(sender, signalIndex, this, memberOffset);
    return true;
}

int QtUiTestSignalReceiver::qt_metacall(QMetaObject::Call call, int methodId, void **a)
{
    methodId = QObject::qt_metacall(call, methodId, a);
    if (methodId < 0)
        return methodId;

    if (call == QMetaObject::InvokeMetaMethod) {
        if (methodId == 0) {
            m_argValues.clear();
            for (int i = 0; i < m_args.count(); ++i) {
                QMetaType::Type type = static_cast<QMetaType::Type>(m_args.at(i));
                m_argValues << QVariant(type, a[i + 1]);
            }
            m_received = true;
            signalReceived();

            if (m_slave) {
                m_slave->signalReceived(m_sender, m_signal, m_argValues);
            }
        }
        --methodId;
    }
    return methodId;
}

void QtUiTestSignalReceiver::signalReceived()
{
}

QtUiTestSignalWaiter::QtUiTestSignalWaiter(QTestSlavePrivate *slave)
    : QtUiTestSignalReceiver(slave), m_elapsed(0)
{
}

bool QtUiTestSignalWaiter::setExpectedSignal(QObject *sender, const char *signal)
{
    clear();
    if (!connectSignal(sender, signal)) {
        return false;
    }
    m_time.start();
    return true;
}

void QtUiTestSignalWaiter::clear()
{
    if (m_sender) {
        QtUiTestConnectionManager::instance()->disconnect(m_sender, -1, this, -1);
        m_sender = 0;
    }
    m_received = false;
    m_args.clear();
    m_argValues.clear();
    m_elapsed = 0;
}

void QtUiTestSignalWaiter::signalReceived()
{
    m_elapsed = m_time.elapsed();

    if (m_eventLoop.isRunning())
        m_eventLoop.quit();
}

bool QtUiTestSignalWaiter::waitForSignal(int timeout)
{
    if (m_received)
        return true;

    QTimer timer;
    timer.setInterval(timeout);
    timer.setSingleShot(true);
    QObject::connect(&timer, SIGNAL(timeout()), &m_eventLoop, SLOT(quit()));
    timer.start();
    m_eventLoop.exec();

    return m_received;
}

QstMessage QTestSlavePrivate::grabImage(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    QImage img;

    Widget *tw = 0;
    if (!message["widgetSpecifier"].toString().isEmpty()) {
        tw = findTestWidget<Widget>(message);
    }

    if (!tw) {
       if (QtUiTest::errorString().isEmpty()) {
            setErrorString(QLatin1String("Widget not found"));
       }
       return reply;
    }

    tw->grabImage(img);
    reply["grabImage"] = img;
    return reply;
}

QstMessage QTestSlavePrivate::window(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    Widget *w = findTestWidget<Widget>(message);
    qDebug() << "Finding window for " << w;
    if (w) {
        QObject *window = w->window();
        qDebug() << "window " << window;
        if (window) {
            QActiveTestWidget::instance()->registerTestWidget(window);
            reply["window"] = QTestSlave::objectToMap(window);
        }
    }

    return reply;
}

QstMessage QTestSlavePrivate::activeWidgetInfo(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    Q_UNUSED(message);
    QstMessage reply;
    QString error;
    if (!QActiveTestWidget::instance()->rescan()) {
        return reply;
    }

    QString ret = QString("applicationIdentifier = '%1'\n\n").arg(QTestWidgets::windowProperty(QLatin1String("applicationIdentifier")).toString());
    ret += QActiveTestWidget::instance()->descendantTree(QTestWidgets::activeWidget()).join(QLatin1String("\n"));

    reply["activeWidgetInfo"] = ret;
    return reply;
}

QstMessage QTestSlavePrivate::getObjects(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    reply["getObjects"] = QActiveTestWidget::instance()->getObjects(message["widgetSpecifier"].toString());
    return reply;
}

QstMessage QTestSlavePrivate::isVisible(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    Widget *w = findTestWidget<Widget>(message);
    if (w) {
        reply["isVisible"] = w->isVisible();
    } else {
        reply["isVisible"] = false;
    }

    return reply;
}

QstMessage QTestSlavePrivate::hasContents(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    QtUiTest::Object *w = findTestWidget<QtUiTest::Object>(message);
    if (w) {
        reply["hasContents"] = w->hasContents();
    } else {
        reply["hasContents"] = false;
    }

    return reply;
}

QstMessage QTestSlavePrivate::getCenter(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    Widget *qw = findTestWidget<Widget>(message);
    if (qw) {
        QPoint pos = qw->center();
        reply["getCenter"] = QVariant::fromValue(pos);
    }

    return reply;
}

QstMessage QTestSlavePrivate::inputMethod(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    Q_UNUSED(message);
    QstMessage reply;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QObject *obj = qGuiApp->inputMethod();
    if (obj) {
        QActiveTestWidget::instance()->registerTestWidget(obj);
        reply["inputMethod"] = QTestSlave::objectToMap(obj);
    } else {
        QtUiTest::setErrorString("Could not find inputMethod");
    }
#else
    QtUiTest::setErrorString("inputMethod() requires Qt 5");
#endif
    return reply;
}

QstMessage QTestSlavePrivate::appName(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    Q_UNUSED(message);
    QstMessage reply;
    reply["appName"] = qApp->applicationName();
    return reply;
}

QstMessage QTestSlavePrivate::applicationData(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    Q_UNUSED(message);
    QstMessage reply;
    QVariantMap appData;
    appData["rootItem"] = QTestSlave::objectToMap(QTestWidgets::activeWidget());
    appData["appObject"] = QTestSlave::objectToMap(qApp);
    appData["appId"] = QTestWidgets::windowProperty(QLatin1String("applicationIdentifier"));
    appData["appName"] = qApp->applicationName();
    reply["applicationData"] = appData;
    return reply;
}

QstMessage QTestSlavePrivate::applicationWindows(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    Q_UNUSED(message);
    QstMessage reply;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QWindow *focusWindow = qApp->focusWindow();

    QVariantList topLevel;
    foreach (QWindow* window, qApp->topLevelWindows()) {
        QActiveTestWidget::instance()->registerTestWidget(window);
        topLevel << QTestSlave::objectToMap(window);
    }
    QVariantList allWindows;
    foreach (QWindow* window, qApp->allWindows()) {
        QActiveTestWidget::instance()->registerTestWidget(window);
        allWindows << QTestSlave::objectToMap(window);
    }

    QVariantMap windowData;
    QActiveTestWidget::instance()->registerTestWidget(focusWindow);
    windowData["focusWindow"] = QTestSlave::objectToMap(qApp->focusWindow());
    windowData["topLevelWindows"] = topLevel;
    windowData["allWindows"] = allWindows;
    reply["applicationWindows"] = windowData;
#endif
    return reply;
}

//FIXME: This should require a window to be specified, should be handled in
// window handler code
QstMessage QTestSlavePrivate::windowProperty(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    Q_UNUSED(message);
    QstMessage reply;
    QString property = message["property"].toString();
    QVariant value = QTestWidgets::windowProperty(property);
    reply["windowProperty"] = value;
    return reply;
}

static QByteArray methodSignature(const QMetaMethod &method)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    return method.methodSignature();
#else
    return QByteArray::fromRawData(method.signature(), qstrlen(method.signature()));
#endif
}

QstMessage QTestSlavePrivate::invokeMethod(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QObject *o;
    QString error;
    QstMessage reply;
    if (!QActiveTestWidget::instance()->findWidget( message["widgetSpecifier"].toString(), o, error))
        return MESSAGE_ERROR(reply,error);

    QString method = message["method"].toString();
    bool returns   = message["returns"].toBool();
    Qt::ConnectionType connType = (Qt::ConnectionType)message["conntype"].toInt();

    method = QMetaObject::normalizedSignature(qPrintable(method));

    QVariantList argList = message["args"].toList();

    QMetaObject const *mo = o->metaObject();

    int m = mo->indexOfMethod(QMetaObject::normalizedSignature(qPrintable(method)));
    if (-1 == m) {
        (void) MESSAGE_ERROR(reply, "ERROR_NO_METHOD");
        reply["warning"] = QString("method '%1' is not valid").arg(method);
        return reply;
    }
    QMetaMethod mm = mo->method(m);
    QList<QByteArray> paramTypes = mm.parameterTypes();
    QList<QByteArray> paramNames = mm.parameterNames();

    if (paramTypes.count() != argList.count()) {
        (void)MESSAGE_ERROR(reply, "ERROR_WRONG_ARG_COUNT");
        reply["warning"] = QString("method '%1' invoked with actual args %2, expected args %3")
                .arg(QString::fromLatin1(methodSignature(mm))).arg(argList.count()).arg(paramTypes.count());
        return reply;
    }
    QString retType = mm.typeName();
    if (returns && retType.isEmpty()) {
        (void)MESSAGE_ERROR(reply, "ERROR_NO_RETURN");
        reply["warning"] = QString("method '%1' does not return a value").arg(QString::fromLatin1(methodSignature(mm)));
        return reply;
    }

    QGenericArgument args[10];
    for (int index = 0;index < argList.count(); index++) {
        int typeId = QMetaType::type(paramTypes[index]);
        if (typeId != QMetaType::QVariant) {
            if (argList[index].canConvert(static_cast<QVariant::Type>(typeId))) {
                argList[index].convert(static_cast<QVariant::Type>(typeId));
            } else {
                (void) MESSAGE_ERROR(reply, "ERROR_IN_INVOKE");
                reply["warning"] = QString("invalid parameter type for parameter %1 (was %2 but expected %3)")
                        .arg(index).arg(argList[index].typeName()).arg(qPrintable(paramTypes[index]));
                return reply;
            }
        }

        args[index] = QGenericArgument(paramTypes[index].constData(), argList[index].constData());
    }

    QString methodName = method.left(method.indexOf('('));
    if (retType != QLatin1String("void")) {
        QVariant returnValue;
        int typeId = QMetaType::type(mm.typeName());

        if (!typeId) {
            (void) MESSAGE_ERROR(reply, "ERROR_IN_INVOKE");
            reply["warning"] = QString("could not look up type id for type %1)")
                    .arg(mm.typeName());
            return reply;
        } else if (typeId != QMetaType::QVariant) {
            returnValue = QVariant(typeId, (const void*)NULL);
        }

        if (!mm.invoke(o, connType,
            QGenericReturnArgument(mm.typeName(), returnValue.data()), args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9])) {
            (void) MESSAGE_ERROR(reply, "ERROR_IN_INVOKE");
            reply["warning"] = QString("invoke of method '%1' with %2 args and a return type of '%3' failed")
                .arg(method).arg(paramTypes.count()).arg(retType);
            return reply;
        }
        reply["returns"] = returnValue;
    } else {
        if (!QMetaObject::invokeMethod(o, qPrintable(methodName), connType,
            args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9])) {
            (void) MESSAGE_ERROR(reply, "ERROR_IN_INVOKE");
            reply["warning"] = QString("invoke of method '%1' with %2 args failed").arg(method).arg(paramTypes.count());
            return reply;
        }
    }

    return reply;
}

QstMessage QTestSlavePrivate::setProperty(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    QtUiTest::Object *w = findTestWidget<Object>(message);
    if (w) {
        QString property = message["property"].toString();
        QVariant value = message["value"];
        if (!w->setProperty(property, value) && QtUiTest::errorString().isEmpty()) {
            // was a dynamic property created?
            if (!w->getProperties().contains(property)) {
                return MESSAGE_ERROR(reply, "ERROR_SETTING_PROPERTY");
            }
        }
    }

    return reply;
}

QstMessage QTestSlavePrivate::getProperty(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    QtUiTest::Object *w = findTestWidget<Object>(message);
    if (w) {
        QString property = message["property"].toString();
        QVariant value = QTestSlave::getObjectProperty(w, property);
        reply["getProperty"] = value;
        if (!value.isValid()) {
            QtUiTest::setErrorString("Failed to retrieve property from object");
        }
    }

    return reply;
}

QVariant QTestSlave::getObjectProperty(QtUiTest::Object *w, const QString &property)
{
    QVariant value;
    if (!w) {
        QtUiTest::setErrorString(QString("Cannot retrieve property '%1' from null object").arg(property));
        return value;
    }
    value = w->getProperty(property);
    if (!value.isValid()) {
        QtUiTest::setErrorString(QString("Failed to retrieve property '%1' from object").arg(property));
        return value;
    }

    return QTestSlave::updateVariant(value);
}

QVariant QTestSlave::updateVariant(const QVariant &value)
{
    if (value.userType() == QMetaType::QObjectStar) {
        if (QObject *obj = value.value<QObject*>()) {
            QActiveTestWidget::instance()->registerTestWidget(obj);
            return QTestSlave::objectToMap(obj);
        } else {
            return QLatin1String("<Null Object>");
        }
    } else if (value.userType() == QVariant::List) {
        QVariantList valueList;
        foreach (const QVariant &item, value.toList()) {
            valueList << QTestSlave::updateVariant(item);
        }
        return valueList;
    } else if (value.type() == QVariant::Map) {
        QVariantMap map = value.toMap();
        QVariantMap::iterator i;
        for (i = map.begin(); i != map.end(); ++i) {
            i.value() = QTestSlave::updateVariant(i.value());
        }
        return map;
    } else if ((int)value.type() == QMetaType::QVariant) {
        return QTestSlave::updateVariant(value.value<QVariant>());
    } else if ((value.type() == QVariant::Hash)
        || (value.type() == QVariant::UserType)
        || (value.type() == QMetaType::VoidStar)) {
        return QString("<%1>").arg(value.typeName());
    }

    return value;
}

QVariantMap QTestSlave::objectToMap(const QObject *obj, const QStringList &properties)
{
    QVariantMap map;
    map["_signature_"] = QTestWidgets::signature(obj);
    QtUiTest::Object *qw = qtuitest_cast<Object*>(obj);
    if (qw) {
        map["_type_"] = qw->typeOf();
        if (!properties.isEmpty()) {
            QVariantMap prefetch;
            foreach (const QString &property, properties) {
                QVariant value = QTestSlave::getObjectProperty(qw, property);
                prefetch[property] = value;
            }
            map["_prefetch_"] = prefetch;
        }
    }
    return map;
}

QstMessage QTestSlavePrivate::getProperties(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    QtUiTest::Object *w = findTestWidget<Object>(message);

    if (!w) {
        QtUiTest::setErrorString(QLatin1String("Unable to find specified widget"));
        return reply;
    }

    QVariantMap map = w->getProperties();
    if (map.isEmpty()) {
        QtUiTest::setErrorString(QLatin1String("Failed to retrieve properties from object"));
    }

    foreach (const QString &key, map.keys()) {
        map[key] = QTestSlave::updateVariant(map[key]);
    }

    reply[QLatin1String("getProperties")] = map;
    return reply;
}

QstMessage QTestSlavePrivate::queryProperties(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    QString error;

    QVariantMap searchValues = message["searchvalues"].toMap();
    if ( searchValues.isEmpty() ) {
        QtUiTest::setErrorString(QLatin1String("ERROR_MISSING_PARAMETERS"));
        return reply;
    }

    int flags = message["flags"].toInt();
    QObjectList ol = QActiveTestWidget::instance()->queryProperties( searchValues, static_cast<QtUiTest::QueryFlags>(flags) );

    if (!QtUiTest::errorString().isEmpty())
        return reply;

    if (!(flags & QtUiTest::NoSort)) {
        // Sort the list by widget position
        qStableSort(ol.begin(), ol.end(), QTestWidgets::lessThan);
    }

    QStringList properties = message["properties"].toStringList();
    QVariantList matches;
    foreach (QObject *o, ol) {
        QActiveTestWidget::instance()->registerTestWidget(o);
        matches << QTestSlave::objectToMap(o, properties);
    }

    reply["queryProperties"] = matches;
    return reply;
}

QstMessage QTestSlavePrivate::getGeometry(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    Widget *w = findTestWidget<Widget>(message);
    if (w) {
        QRect ret = w->geometry();
        reply["getGeometry"] = ret;
    }

    return reply;
}

void QTestSlavePrivate::resetBenchmarks()
{
#ifdef QTUITEST_CPU_TICKS
    startTicks = getticks();
#endif
}

qint64 QTestSlavePrivate::cpuTicks()
{
#ifdef QTUITEST_CPU_TICKS
    CycleCounterTicks now = getticks();
    return qRound64(elapsed(now, startTicks));
#else
    return -1;
#endif
}

Q_DECLARE_METATYPE(QVariant);

void QTestSlave::processMessage( QstMessage *msg )
{
    QTT_TRACE_FUNCTION();
    QtUiTest::setErrorString(QString());
    QstMessage reply = constructReplyToMessage(*msg);
    if (reply.state() != QstMessage::MessageEmpty) {
        replyMessage(msg, reply);
    }
}

void QTestSlave::registerHandler(QtUiTest::MessageHandler *handler)
{
    m_messageHandlers[handler->handlerName()] = handler;
    qDebug() << "registered handler " << handler->handlerName() << handler;
}

void QTestSlave::refreshPlugins()
{
    QTT_TRACE_FUNCTION();

    QSet<QString> pluginsToLoad;
    QList<QDir> pluginDirs;

    const QString pluginType("qtuitest");
    QSet<QString> libPaths = QCoreApplication::libraryPaths().toSet();

    foreach (const QString& libPath, libPaths) {
        QDir dir(libPath + "/" + pluginType);
        if (!dir.exists()) {
            continue;
        }
        pluginDirs << dir;
    }

    foreach (QDir const& dir, pluginDirs) {
        foreach (const QString& file, dir.entryList(QDir::Files|QDir::NoDotAndDotDot)) {
            QString filename = dir.canonicalPath() + "/" + file;
            if (!QLibrary::isLibrary(filename)) continue;
            pluginsToLoad << filename;
        }
    }
    QPluginLoader pluginLoader;
    QSet<QString> lastPluginsToLoad;
    QStringList errors;

    // dumb dependency handling: keep trying to load plugins until we
    // definitely can't progress.
    while (lastPluginsToLoad != pluginsToLoad) {
        lastPluginsToLoad = pluginsToLoad;
        errors.clear();
        foreach (const QString& plugin, pluginsToLoad) {
            pluginLoader.setFileName(plugin);
            // enable RTLD_GLOBAL, so plugins can access each other's symbols.
            pluginLoader.setLoadHints(QLibrary::ExportExternalSymbolsHint);
            pluginLoader.load();

            QObject *instance = pluginLoader.instance();
            QString error;
            if (!instance)
                error = "cannot resolve 'qt_plugin_instance': " + pluginLoader.errorString();

            QtUiTest::MessageHandler *handler = qobject_cast<QtUiTest::MessageHandler*>(instance);
            if (!handler) {
                if (error.isEmpty()) error = pluginLoader.errorString();
                QString formattedError;
                QDebug(&formattedError)
                    << "QtUitest: failed to load qtuitest widgets plugin"
                    << "\n   plugin"   << plugin
                    << "\n   instance" << instance
                    << "\n   error"    << error;
                errors << formattedError;
            } else {
                pluginsToLoad -= plugin;
                registerHandler(handler);
            }
        }
    }

    foreach (const QString& error, errors)
        qWarning() << qPrintable(error);
}

QstMessage QTestSlave::constructReplyToMessage( const QstMessage &_msg )
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    QstMessage msg(_msg);

// TO HANDLE A NEW SYSTEM TEST EVENT:
//  implement a function with signature:
//  QstMessage QTestSlavePrivate::eventName(const QstMessage &message)

    QString event = msg.event();
    int pos = event.lastIndexOf(QChar('/'));
    if (pos != -1) {
        QtUiTest::MessageHandler *handler = m_messageHandlers[event.left(pos)];
        if (handler) {
            msg.setEvent(event.mid(pos+1));
            reply = handler->handleMessage(msg);
        }
        // TODO: Ensure correct plugin is loaded
        // and call constructReplyToMessage there (handleMessage?)
    } else {
        d->resetBenchmarks();
        if (!QMetaObject::invokeMethod(d, msg.event().toLatin1().constData(), Qt::DirectConnection,
                                        Q_RETURN_ARG(QstMessage, reply),
                                        Q_ARG(QstMessage, msg))) {
            (void)MESSAGE_ERROR(reply, "ERROR: Unhandled event '" + msg.event() + "'");
        }

#ifdef QTUITEST_CPU_TICKS
        reply["cpuTicks"] = d->cpuTicks();
#endif
    }

    if (!QtUiTest::debugOutput().isEmpty()) {
        reply["debug"] = QtUiTest::debugOutput();\
    }

    if (!QtUiTest::errorString().isEmpty()) {
        reply.setState(QstMessage::MessageError);
        return MESSAGE_ERROR(reply, QtUiTest::errorString());
    }

    if (reply.state() == QstMessage::MessageEmpty) {
        reply.setState(QstMessage::MessageSuccess);
    }
    return reply;
}

/*
  Return true if first item is higher than second item, based on z-order.
  This does not mean that the higher item obscures the other, as they may
  not be overlapping, or the higher item may be transparent.

  See http://doc.qt.nokia.com/qt5-snapshot/qml-qtquick2-item.html#z-prop

  * Find nearestCommonAncestor (common ancestor closest to both items)

    Neither item is ancestor of other ¦ One item is ancestor of another
                                      ¦
    root                              ¦  root
    +-- ...                           ¦  +-- ...
        +-- nearestCommonAncestor     ¦      +-- o1 == keyAncestor1 *
            +-- ...                   ¦          +-- keyAncestor2
            +-- keyAncestor1          ¦              +-- ...
            ¦   +-- ...               ¦                  +-- o2
            ¦       +-- o1            ¦
            +-- keyAncestor2          ¦  * nearestCommonAncestor == o1
                +-- ...               ¦
                    +-- o2            ¦

  * There should always be at least one common ancestor
  * The nearestCommonAncestor may be o1 or o2 (if one item is an ancestor
    of the other). If nearestCommonAncestor == o1 then keyAncestor1 == o1,
    (and similarly for o2)
  * z1 is keyAncestor1's z property, z2 == keyAncestor2's z property
  * If neither item is an ancestor of the other, then keyAncestors will be
    siblings. Their z values (z1 and z2) are compared:
      (z1 > z2)  -->  o1 is above o2 (return true)
      (z1 < z2)  -->  o2 is above o1 (return false)
    If (z1 == z2), use the position of each keyAncestor in the
    nearestCommonAncestor's child list (later sibling is above the earlier)
  * If o1 is an ancestor of o2, then o2 will be above o1 UNLESS z2 is negative
  * If o2 is an ancestor of o1, then o1 will be above o2 UNLESS z1 is negative

*/
QstMessage QTestSlavePrivate::isHigher( const QstMessage &message )
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    QString error;

    QString sig1 = message["widgetSpecifier"].toString();
    QObject *o1;
    if (!QActiveTestWidget::instance()->findWidget( sig1, o1, error)) {
        return reply;
    }
    QtUiTest::Object *w1 = qtuitest_cast<QtUiTest::Object*>(o1);

    QString sig2 = message["otherwidget"].toString();
    QObject *o2;
    if (!QActiveTestWidget::instance()->findWidget( sig2, o2, error)) {
        return reply;
    }
    QtUiTest::Object *w2 = qtuitest_cast<QtUiTest::Object*>(o2);

    if (!(w1 && w2)) {
        if (!QtUiTest::errorString().isEmpty())
            return reply;
        QtUiTest::setErrorString(QLatin1String("Widget not found"));
        return reply;
    }

    QObjectList ancestors1 = w1->ancestors();
    QObjectList ancestors2 = w2->ancestors();
    ancestors1.prepend(o1);
    ancestors2.prepend(o2);

    QObject *nearestCommonAncestor = 0;
    QObject *keyAncestor1 = 0;
    QObject *keyAncestor2 = 0;
    int al1 = ancestors1.length();
    int al2 = ancestors2.length();
    int alength = ((al1 > al2) ? al2 : al1);

    // Work from end of list (root item) to start (the item)
    for (int i=0; i< alength; i++) {
        QObject *a1 = ancestors1.takeLast();
        QObject *a2 = ancestors2.takeLast();
        if (a1 == a2) {
            nearestCommonAncestor = a1;
        } else {
            keyAncestor1 = a1;
            keyAncestor2 = a2;
            break;
        }
    }
    if (keyAncestor1 == 0) {
        if (ancestors1.isEmpty()) {
            keyAncestor1 = o1;
        } else {
            keyAncestor1 = ancestors1.takeLast();
        }
    }
    if (keyAncestor2 == 0) {
        if (ancestors2.isEmpty()) {
            keyAncestor2 = o2;
        } else {
            keyAncestor2 = ancestors2.takeLast();
        }
    }
    if (nearestCommonAncestor == 0) {
        // no common ancestor, this should not be possible
        reply["ishigher"] = false;
        QtUiTest::setErrorString(QLatin1String("Cannot find a common ancestor item"));
        return reply;
    }

    bool ok;
    double z1 = keyAncestor1->property("z").toDouble(&ok);
    if (!ok) {
        z1 = 0;
    }

    double z2 = keyAncestor2->property("z").toDouble(&ok);
    if (!ok) {
        z2 = 0;
    }

    QtUiTest::Object *nca = qtuitest_cast<QtUiTest::Object*>(nearestCommonAncestor);
    bool ret = false;
    if (nearestCommonAncestor == keyAncestor1) {
        // first item is ancestor of second item, so can only be
        // above it if z2 < 0
        ret = (z2 < 0);
    } else if (nearestCommonAncestor == keyAncestor2) {
        // second item is ancestor of first item
        ret = (z1 >= 0);
    } else if (z1 > z2) {
        ret = true;
    } else if (z1 == z2) {
        // The z values are equal, so which item is on top depends
        // on the position in list of children
        QObjectList ncaChildren = nca->children();
        ret = (ncaChildren.indexOf(keyAncestor1) > ncaChildren.indexOf(keyAncestor2));
    }

    reply["isHigher"] = ret;
    return reply;
}

QstMessage QTestSlavePrivate::enterText(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    if (!message["text"].isValid()) {
        return MESSAGE_ERROR(reply, "ERROR_MISSING_PARAMETERS");
    }
    QString text = message["text"].toString();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QPlatformInputContext *ic = QGuiApplicationPrivate::platformIntegration()->inputContext();
    foreach (const QChar& c, text) {
         Qt::KeyboardModifiers modifiers = asciiToModifiers(c.toLatin1());
         QKeyEvent pressEvent(QEvent::KeyPress, c.unicode(), modifiers, QString(c), false, 1);
         QKeyEvent releaseEvent(QEvent::KeyRelease, c.unicode(), modifiers, QString(c), false, 1);
         QMetaObject::invokeMethod(ic, "sendKeyEvent", Qt::DirectConnection, Q_ARG(QKeyEvent*, &pressEvent));
         QMetaObject::invokeMethod(ic, "sendKeyEvent", Qt::DirectConnection, Q_ARG(QKeyEvent*, &releaseEvent));
    }
#endif
    return reply;
}

QstMessage QTestSlavePrivate::keyPress(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    if (!message["key"].isValid()) {
        return MESSAGE_ERROR(reply, "ERROR_MISSING_KEY");
    }

    Qt::Key key = QtUiTest::variantToKey(message["key"]);

    if (!key) return MESSAGE_ERROR(reply, "ERROR_ZERO_KEY");

    QtUiTest::keyPress(key);

    if (message["duration"].isValid()) {
        int duration = message["duration"].toInt();
        if (duration >= 500) {
            QtUiTest::wait(500);
            duration -= 500;
            bool keyRepeat=QtUiTest::testInputOption(QtUiTest::KeyRepeat);
            QtUiTest::setInputOption(QtUiTest::KeyRepeat, true);
            QtUiTest::keyPress(key);
            while (duration > 0) {
                QtUiTest::wait(150);
                duration -= 150;
                QtUiTest::keyPress(key);
            }
            QtUiTest::setInputOption(QtUiTest::KeyRepeat, keyRepeat);
        } else {
            QtUiTest::wait(duration);
        }
    }

    return reply;
}

QstMessage QTestSlavePrivate::keyRelease(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    if (!message["key"].isValid()) {
        return MESSAGE_ERROR(reply, "ERROR_MISSING_KEY");
    }

    Qt::Key key = QtUiTest::variantToKey(message["key"]);

    if (!key) return MESSAGE_ERROR(reply, "ERROR_ZERO_KEY");
    QtUiTest::keyRelease(key);
    return reply;
}

QstMessage QTestSlavePrivate::keyClick(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    if (!message["key"].isValid()) {
        return MESSAGE_ERROR(reply, "ERROR_MISSING_KEY");
    }

    Qt::Key key = QtUiTest::variantToKey(message["key"]);

    if (!key) return MESSAGE_ERROR(reply, "ERROR_ZERO_KEY");
    QtUiTest::keyClick(key);
    return reply;
}

QstMessage QTestSlavePrivate::touchPress(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;

    QVariantList points(touchPointsForMessage(message));
    if (!QtUiTest::errorString().isEmpty())
        return reply;

    QtUiTest::touchPress(points);
    return reply;
}

QstMessage QTestSlavePrivate::touchUpdate(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;

    QVariantList points(touchPointsForMessage(message));
    if (!QtUiTest::errorString().isEmpty())
        return reply;

    QtUiTest::touchUpdate(points);
    return reply;
}

QstMessage QTestSlavePrivate::touchRelease(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;

    QVariantList points(touchPointsForMessage(message));
    if (!QtUiTest::errorString().isEmpty())
        return reply;

    QtUiTest::touchRelease(points);
    return reply;
}


QstMessage QTestSlavePrivate::mousePress(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    bool ok;
    Qt::MouseButtons buttons = (Qt::MouseButtons)(message["buttons"].toInt(&ok));
    if (!ok) buttons = Qt::LeftButton;

    QPoint pos(mousePointForMessage(message));
    if (!QtUiTest::errorString().isEmpty())
        return reply;

    QtUiTest::mousePress(pos, buttons);
    return reply;
}

QstMessage QTestSlavePrivate::mouseRelease(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    bool ok;
    Qt::MouseButtons buttons = (Qt::MouseButtons)(message["buttons"].toInt(&ok));
    if (!ok) buttons = Qt::LeftButton;

    QPoint pos(mousePointForMessage(message));
    if (!QtUiTest::errorString().isEmpty())
        return reply;

    QtUiTest::mouseRelease(pos, buttons);
    return reply;
}

QstMessage QTestSlavePrivate::mouseClick(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    bool ok;
    Qt::MouseButtons buttons = (Qt::MouseButtons)(message["buttons"].toInt(&ok));
    if (!ok) buttons = Qt::LeftButton;

    QPoint pos(mousePointForMessage(message));
    if (!QtUiTest::errorString().isEmpty())
        return reply;

    QtUiTest::mouseClick(pos, buttons);
    return reply;
}

QstMessage QTestSlavePrivate::mouseDoubleClick(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    bool ok;
    Qt::MouseButtons buttons = (Qt::MouseButtons)(message["buttons"].toInt(&ok));
    if (!ok) buttons = Qt::LeftButton;

    QPoint pos(mousePointForMessage(message));
    if (!QtUiTest::errorString().isEmpty())
        return reply;

    QtUiTest::mouseDClick(pos, buttons);
    return reply;
}

QstMessage QTestSlavePrivate::mouseMove(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    QPoint pos(mousePointForMessage(message));

    if (!QtUiTest::errorString().isEmpty())
        return reply;

    QtUiTest::mouseMove(pos);
    return reply;
}

QVariantList QTestSlavePrivate::touchPointsForMessage(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    if (message["points"].isValid())
        return message["points"].toList();

    QVariantList ret;
    Widget *w = findTestWidget<Widget>(message);
    if (w)
         ret += w->geometry().center();

    return ret;
}

QPoint QTestSlavePrivate::mousePointForMessage(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    if (message["pos"].isValid())
        return message["pos"].value<QPoint>();

    QPoint ret;
    Widget *w = findTestWidget<Widget>(message);
    if (w) {
         ret = w->geometry().center();
    }

    return ret;
}

QstMessage QTestSlavePrivate::translate( const QstMessage &message )
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    QByteArray context = message["context"].toString().toLatin1();
    QByteArray text = message["text"].toString().toLatin1();
    QByteArray comment = message["comment"].toString().toLatin1();
    int n = message["number"].toInt();
    reply["translate"] = QCoreApplication::translate(context, text, comment, QCoreApplication::CodecForTr, n);
    return reply;
}

QstMessage QTestSlavePrivate::trId( const QstMessage &message )
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    QByteArray id = message["id"].toString().toLatin1();
    int n = message["number"].toInt();
    reply["trId"] = qtTrId(id, n);
    return reply;
}

QstMessage QTestSlavePrivate::getLocale( const QstMessage &message )
{
    QTT_TRACE_FUNCTION();
    Q_UNUSED(message);
    QstMessage reply;
    reply["getLocale"] = QLocale::system();
    return reply;
}

QstMessage QTestSlavePrivate::setConfig(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    Q_UNUSED(message);
    QstMessage reply;
    if (message["autoScan"].isValid()) {
        QActiveTestWidget::instance()->setAutoScan(message["autoScan"].toBool());
    }

    return reply;
}

QstMessage QTestSlavePrivate::rescan(const QstMessage &message)
{
    QTT_TRACE_FUNCTION();
    Q_UNUSED(message);
    QstMessage reply;
    QString error;
    QActiveTestWidget::instance()->forceRescan();
    return reply;
}

QstMessage QTestSlavePrivate::typeOf( const QstMessage &message )
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    QtUiTest::Object *w = findTestWidget<QtUiTest::Object>(message);
    if (w) {
        reply["typeOf"] = w->typeOf();
    }
    return reply;
}

QstMessage QTestSlavePrivate::inheritance( const QstMessage &message )
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    QtUiTest::Object *w = findTestWidget<QtUiTest::Object>(message);
    if (w) {
        reply["inheritance"] = w->inheritance();
    }
    return reply;
}

QstMessage QTestSlavePrivate::queuedMessage( const QstMessage &message )
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;
    bool ok;
    int count = message["count"].toInt(&ok);
    if (ok) {
        for (int i=0; i<count; i++) {
            QString event = message[QString("event_%1").arg(i)].toString();
            QVariantMap map = message[QString("map_%1").arg(i)].toMap();
            if (event == "wait") {
                QtUiTest::wait(map["delay"].toInt());
            } else {
                QstMessage msg(event, map);
                QstMessage reply = p->constructReplyToMessage(msg);
                if (reply.state() != QstMessage::MessageSuccess) {
                    return reply;
                }
            }
        }
    }

    return reply;
}

QstMessage QTestSlavePrivate::getMethods(const QstMessage &message)
{
    QObject *o;
    QString error;
    QstMessage reply;
    if (!QActiveTestWidget::instance()->findWidget( message["widgetSpecifier"].toString(), o, error))
        return MESSAGE_ERROR(reply, error);

    QMetaObject const *mo = o->metaObject();
    int count = mo->methodCount();
    QVariantList methods;
    for (int i=0; i<count; i++) {
        QMetaMethod mm = mo->method(i);
        QVariantMap methodMap;
        methodMap["signature"] = methodSignature(mm);
        methodMap["methodType"] = mm.methodType();
        QStringList paramNames;
        foreach (const QByteArray &param, mm.parameterNames()) {
            paramNames << param;
        }
        if (!paramNames.isEmpty()) {
            methodMap["paramNames"] = paramNames;
        }
        QStringList paramTypes;
        foreach (const QByteArray &paramType, mm.parameterTypes()) {
            paramTypes << paramType;
        }
        if (!paramTypes.isEmpty()) {
            methodMap["paramTypes"] = paramTypes;
        }
        QString typeName(mm.typeName());
        if (!typeName.isEmpty()) {
            methodMap["returnType"] = typeName;
        }
        methods << methodMap;
    }

    reply[QLatin1String("getMethods")] = methods;
    return reply;
}

QstMessage QTestSlavePrivate::expectSignal( const QstMessage &message )
{
    QstMessage reply;
    QString error;

    QString object = message["widgetSpecifier"].toString();
    QString signal = message["signal"].toString();
    if (object.isEmpty() || signal.isEmpty()) {
        return MESSAGE_ERROR(reply, "ERROR_MISSING_PARAMETERS");
    }

    QObject *o;
    if (!QActiveTestWidget::instance()->findWidget(object, o, error))
        return MESSAGE_ERROR(reply, error);
    QMetaObject const *mo = o->metaObject();
    int m = mo->indexOfMethod(QMetaObject::normalizedSignature(qPrintable(signal)));
    QMetaMethod mm = mo->method(m);
    if (mm.methodType() != QMetaMethod::Signal) {
        return MESSAGE_ERROR(reply, "Method is not a signal");
    }

    if (!m_signalWaiter.setExpectedSignal(o, signal.toLatin1())) {
        return MESSAGE_ERROR(reply, "Unable to connect signal");
    }

    return reply;
}

QstMessage QTestSlavePrivate::waitForSignal( const QstMessage &message )
{
    QstMessage reply;
    QString error;

    int timeout = message["timeout"].toInt();
    bool received = m_signalWaiter.waitForSignal(timeout);

    if (!received) {
        return MESSAGE_ERROR(reply, "Signal not received");
    }

    reply["args"] = QTestSlave::updateVariant(m_signalWaiter.argValues());
    reply["elapsed"] = m_signalWaiter.elapsed();
    return reply;
}

QstMessage QTestSlavePrivate::connectSignal( const QstMessage &message )
{
    QstMessage reply;
    QString error;

    QString object = message["widgetSpecifier"].toString();
    QString signal = message["signal"].toString();
    if (object.isEmpty() || signal.isEmpty()) {
        return MESSAGE_ERROR(reply, "ERROR_MISSING_PARAMETERS");
    }

    QObject *o;
    if (!QActiveTestWidget::instance()->findWidget(object, o, error))
        return MESSAGE_ERROR(reply, error);
    QMetaObject const *mo = o->metaObject();
    int m = mo->indexOfMethod(QMetaObject::normalizedSignature(qPrintable(signal)));
    QMetaMethod mm = mo->method(m);
    if (mm.methodType() != QMetaMethod::Signal) {
        return MESSAGE_ERROR(reply, "Method is not a signal");
    }

    ReceiverKey key(o, signal);
    if (m_signalReceivers.contains(key)) {
        // Already connected to this signal
        return reply;
    }
    QtUiTestSignalReceiver* signalReceiver = new QtUiTestSignalReceiver(this);
    m_signalReceivers[key] = signalReceiver;

    if (!signalReceiver->connectSignal(o, signal.toLatin1())) {
        delete signalReceiver;
        return MESSAGE_ERROR(reply, "Unable to connect signal");
    }

    connect(o, SIGNAL(destroyed(QObject*)), this, SLOT(disconnectAllSignals(QObject*)), Qt::DirectConnection);
    return reply;
}

QstMessage QTestSlavePrivate::disconnectSignal( const QstMessage &message )
{
    QstMessage reply;
    QString error;

    QString object = message["widgetSpecifier"].toString();
    QString signal = message["signal"].toString();
    if (object.isEmpty()) {
        return MESSAGE_ERROR(reply, "ERROR_MISSING_PARAMETERS");
    }

    QObject *o;
    if (!QActiveTestWidget::instance()->findWidget(object, o, error))
        return MESSAGE_ERROR(reply, error);

    if (signal.isEmpty()) {
        disconnectAllSignals(o);
        return reply;
    }

    ReceiverKey key(o, signal);
    if (!m_signalReceivers.contains(key)) {
        return MESSAGE_ERROR(reply, "Not connected to signal");
    }

    delete m_signalReceivers[key];
    m_signalReceivers.remove(key);
    return reply;
}

void QTestSlavePrivate::disconnectAllSignals(QObject *o)
{
    foreach (ReceiverKey key, m_signalReceivers.keys()) {
        if (key.first == o) {
            delete m_signalReceivers[key];
            m_signalReceivers.remove(key);
        }
    }
}

QstMessage QTestSlavePrivate::cleanup( const QstMessage &message )
{
    Q_UNUSED(message);

    foreach (QtUiTestSignalReceiver *receiver, m_signalReceivers.values()) {
        delete receiver;
    }
    m_signalReceivers.clear();
    QstMessage reply;
    return reply;
}

void QTestSlavePrivate::signalReceived(QObject *sender, const QString &signal, const QVariantList &argValues)
{
    QstMessage msg("SIGNAL_RECEIVED");
    msg["signal"] = signal;
    msg["sender"] = QTestSlave::updateVariant(QVariant::fromValue(sender));
    msg["args"] = QTestSlave::updateVariant(argValues);
    p->postMessage( msg );
}

/*
QstMessage QTestSlavePrivate::setSensorMode( const QstMessage &message )
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;

    QByteArray sensorId = message["sensorId"].toByteArray();
    QtUiTestSensorBackend::Mode mode = (QtUiTestSensorBackend::Mode)message["sensorMode"].toInt();

    QtUiTestSensorBackend::setMode(sensorId, mode);

    return reply;
}

QstMessage QTestSlavePrivate::getSensorMode( const QstMessage &message )
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;

    QByteArray sensorId = message["sensorId"].toByteArray();
    QtUiTestSensorBackend::Mode mode = QtUiTestSensorBackend::mode(sensorId);
    reply["sensorMode"] = (int)mode;

    return reply;
}

QstMessage QTestSlavePrivate::queueSensorValues( const QstMessage &message )
{
    QTT_TRACE_FUNCTION();
    QstMessage reply;

    QByteArray sensorId = message["sensorId"].toByteArray();
    QList<QVariant> values = message["sensorValues"].toList();

    QtUiTestSensorBackend::queueTestData(sensorId, values);

    return reply;
}

QstMessage QTestSlavePrivate::getSensorValues( const QstMessage &message )
{
    QstMessage reply;

    QByteArray sensorId = message["sensorId"].toByteArray();

    reply["values"] = QVariant(QtUiTestSensorBackend::testData(sensorId));

    return reply;
}
*/
