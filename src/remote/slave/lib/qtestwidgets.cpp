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


#include <qtestwidgets.h>
#include <QtSystemTest/QstDebug>
#include <QRect>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    #include <QGuiApplication>
    #include <QWindow>
#else
    #include <QApplication>
    #include <QWidget>
#endif

#include <QPlatformNativeInterface>

#define qLog() if (1); else QtUiTest::debug()

uint qHash(QPointer<QObject> o)
{ return qHash((QObject*)o); }

/*!
    Returns a unique string identifier for \a object.
*/
QString QTestWidgets::signature( const QObject *o )
{
    if (!o) {
        return QLatin1String("ERROR_NO_OBJECT");
    }
    QString ret = QActiveTestWidget::instance()->signature(o);
    if (!ret.isEmpty()) {
        return ret;
    }

    QtUiTest::Object* w = qtuitest_cast<QtUiTest::Object*>(o);
    QString typeName;
    if (w) {
        typeName = w->typeOf();
    } else {
        typeName = o->metaObject()->className();
    }

    return QString("%1[%2]").arg(typeName).arg((long)(void*)o,0,32).replace(QLatin1Char(' '), QLatin1Char('_'));
}

/*!
    Returns the testWidget that is associated with the given \a signature.
*/
QObject *QTestWidgets::testWidget( const QString &signature )
{
    QTT_TRACE_FUNCTION();
    return QActiveTestWidget::instance()->testWidget(signature);
}

QObject *QActiveTestWidget::testWidget( const QString &signature )
{
    QTT_TRACE_FUNCTION();
    QObject *rec = d->testWidgetsBySignature.value( signature, 0 );
    return rec;
}

QString QActiveTestWidget::signature( const QObject *o ) const
{
    return d->signaturesByObject.value(o);
}

void QActiveTestWidget::registerTestWidget( QObject *ao )
{
    QTT_TRACE_FUNCTION();
    QString new_signature = QTestWidgets::signature( ao );
    if (d->testWidgetsBySignature.contains(new_signature)) {
        return;
    }
    d->testWidgetsBySignature[new_signature] = ao;
    d->signaturesByObject[ao] = new_signature;
    connect( ao, SIGNAL(destroyed(QObject*)), this, SLOT(onDestroyed(QObject*)), Qt::DirectConnection );
}

void QActiveTestWidget::onDestroyed(QObject *o)
{
    QTT_TRACE_FUNCTION();
    if (o) {
        d->removeTestWidget(o);
    }
}

QVariant QTestWidgets::windowProperty(const QString &property)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QTT_TRACE_FUNCTION();
    QWindow *rootWindow = qobject_cast<QWindow*>(activeWidget());
    if (qGuiApp && rootWindow && qGuiApp->platformNativeInterface() && rootWindow->handle()) {
        return qGuiApp->platformNativeInterface()->windowProperty(rootWindow->handle(), property);
    }
#endif
    return QVariant();
}

/*!
    Returns the current activeWindow, activePopupWidget or activeModalWidget.
*/
QObject* QTestWidgets::activeWidget()
{
    QTT_TRACE_FUNCTION();
    QObject *w = 0;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    w = qApp->focusWindow();

    if (!w && !qApp->topLevelWindows().isEmpty())
        w = qApp->topLevelWindows().first();
#else
    if (w == 0) w = qApp->activeModalWidget();
    if (w == 0) w = qApp->activePopupWidget();
    if (w == 0) w = qApp->activeWindow();
    if (w == 0) w = qApp->focusWidget();
#endif

    return w;
}

QStringList QTestWidgets::objectListToSignatures(const QObjectList& ol)
{
    QTT_TRACE_FUNCTION();
    QStringList ret;
    foreach (QObject *obj, ol) {
        ret << QTestWidgets::signature(obj);
    }
    return ret;
}


QActiveTestWidgetData::QActiveTestWidgetData()
{
    QTT_TRACE_FUNCTION();
}

QActiveTestWidgetData::~QActiveTestWidgetData()
{
    QTT_TRACE_FUNCTION();
}

void QActiveTestWidgetData::clear()
{
    QTT_TRACE_FUNCTION();
    testWidgetsBySignature.clear();
    signaturesByObject.clear();
    m_scannedItems.clear();
}

void QActiveTestWidgetData::removeTestWidget( QObject *w )
{
    QTT_TRACE_FUNCTION();
    m_scannedItems.remove(w);
    testWidgetsBySignature.remove( QTestWidgets::signature(w) );
    signaturesByObject.remove(w);
}

bool QActiveTestWidgetData::scan( QObject *ao, QtUiTest::QueryFlags flags )
{
    QTT_TRACE_FUNCTION();
    if (!ao) return false;

    QActiveTestWidget::instance()->registerTestWidget(ao);
    QtUiTest::Object* aw = qtuitest_cast<QtUiTest::Object*>(ao);
    if (aw == 0) return false;
    bool any_appended = false;
    foreach(QObject *o, aw->children()) {
        QActiveTestWidget::instance()->registerTestWidget(o);

        QtUiTest::Widget* qw = qtuitest_cast<QtUiTest::Widget*>(o);

        if (!qw->isVisible() && !(flags & QtUiTest::ScanInvisible)) {
            continue;
        }

        if (!qw->ignoreScan() || (flags & QtUiTest::ScanDisabledItems)) {
            // Recursively scan child widgets
            any_appended |= scan(o, flags);
        }

        if (!m_scannedItems.contains(o)) {
            any_appended = true;
            m_scannedItems.insert(o);
        }
    }

    return any_appended;
}


QActiveTestWidget::QActiveTestWidget()
{
    QTT_TRACE_FUNCTION();
    d = new QActiveTestWidgetData();
    auto_scan = true;
}

QActiveTestWidget::~QActiveTestWidget()
{
    QTT_TRACE_FUNCTION();
    delete d;
}

QActiveTestWidget* QActiveTestWidget::instance()
{
    static QActiveTestWidget qatw;
    return &qatw;
}

void QActiveTestWidget::clear()
{
    QTT_TRACE_FUNCTION();
    if (d == 0) {
        d = new QActiveTestWidgetData();
    } else
        d->clear();
}

const QLatin1String QActiveTestWidget::NoActiveWidgetError("ERROR: No active widget available");

bool QActiveTestWidget::rescan(QtUiTest::QueryFlags flags)
{
    QTT_TRACE_FUNCTION();
    if (!auto_scan) return true;

    if (d == 0) d = new QActiveTestWidgetData();
    d->m_scannedItems.clear();

    QObject *activeWidget = QTestWidgets::activeWidget();
    if (activeWidget == 0) {
        QtUiTest::setErrorString(NoActiveWidgetError);
        return false;
    }

    d->scan(qApp, flags);
    return true;
}

QStringList QActiveTestWidget::descendantTree(QObject *obj)
{
    QStringList ret;
    QtUiTest::Object *tw = qtuitest_cast<QtUiTest::Object*>(obj);
    if (!tw) {
        return ret;
    }
    ret << QString("%1 [ %2 ]").arg(QTestWidgets::signature(obj)).arg(tw->debugString());
    QObjectList children = tw->children();
    int length = children.length();
    for (int i=0; i<length; ++i) {
        QObject *child = children[i];
        QStringList childTree = descendantTree(child);
        ret << QString("+- %1").arg(childTree.takeFirst());
        foreach (const QString &row, childTree) {
            if (i < length-1) {
                ret << QString("|  %1").arg(row);
            } else {
                ret << QString("   %1").arg(row);
            }
        }
    }
    return ret;
}

void QActiveTestWidget::setAutoScan(bool doScan)
{
    auto_scan = doScan;
}

bool QActiveTestWidget::forceRescan()
{
    bool prev = auto_scan;
    auto_scan = true;
    bool ret = rescan();
    auto_scan = prev;
    return ret;
}

bool QActiveTestWidget::findWidget( const QString &signature, QObject *&buddy, QString &error )
{
    QTT_TRACE_FUNCTION();
    if (signature.isEmpty()) {
        error = QLatin1String("ERROR: No widget specified\n");
        return false;
    }
    buddy = QTestWidgets::testWidget(signature);
    return (buddy != 0);
}

/*!
    Returns a QVariantMap containing details of object specified by \a signature
    and descendants. This is used by the Object Browser.
*/
QVariantMap QActiveTestWidget::getObjects(const QString &signature)
{
    QTT_TRACE_FUNCTION();
    QVariantMap map;
    QObject *baseObject = QTestWidgets::testWidget(signature);

    //FIXME: For now:
    baseObject = qApp;

    if (!baseObject) {
        return map;
    }

    QtUiTest::Object *tw = qtuitest_cast<QtUiTest::Object*>(baseObject);
    if (!tw) {
        return map;
    }

    QObjectList objects;
    objects << baseObject << tw->descendants();

    QVariantMap idMap;
    QVariantMap geometryMap;
    QVariantMap childMap;
    QVariantMap typeMap;
    QVariantMap objectNameMap;
    QVariantMap visibleMap;
    QVariantMap textMap;
    QVariantMap windowMap;

    foreach (QObject *obj, objects) {
        QActiveTestWidget::instance()->registerTestWidget(obj);
        QtUiTest::Object *to = qtuitest_cast<QtUiTest::Object*>(obj);

        if (to) {
            QString sig = QTestWidgets::signature(obj);
            QObjectList children = to->children();
            if (!children.isEmpty()) {
                childMap[sig] = QTestWidgets::objectListToSignatures(to->children());
            }
            objectNameMap[sig] = to->getProperty("objectName");
            typeMap[sig] = to->typeOf();
            QtUiTest::Widget *tw = qtuitest_cast<QtUiTest::Widget*>(obj);
            if (tw) {
                geometryMap[sig] = tw->geometry();
                idMap[sig] = to->getProperty("QMLID");
                visibleMap[sig] = to->getProperty("visible");
                textMap[sig] = to->getProperty("text");
                windowMap[sig] = QTestWidgets::signature(tw->window());
            }
        }
    }

    map["geometry"] = geometryMap;
    map["qmlid"] = idMap;
    map["children"] = childMap;
    map["type"] = typeMap;
    map["objectName"] = objectNameMap;
    map["visible"] = visibleMap;
    map["text"] = textMap;
    map["window"] = windowMap;

    return map;
}

const QLatin1String QActiveTestWidget::childOfProperty("CHILD_OF");
const QLatin1String QActiveTestWidget::descendantOfProperty("DESCENDANT_OF");
const QLatin1String QActiveTestWidget::ancestorOfProperty("ANCESTOR_OF");
const QLatin1String QActiveTestWidget::parentOfProperty("PARENT_OF");
const QLatin1String QActiveTestWidget::resourceOfProperty("RESOURCE_OF");

QObjectList QActiveTestWidget::findObjectsByProperty( const QString &property, const QVariant &searchValue, const QObjectList &searchList )
{
    QTT_TRACE_FUNCTION();
    QObjectList foundList;
    QString propertyUpper = property.toUpper();

    bool ok;
    PropertyMatcher matcher(searchValue, ok);
    if (!ok) {
        QtUiTest::setErrorString(QString("Unable to match for property '%1'").arg(property));
        return foundList;
    }

    foreach (QObject *obj, searchList) {
        QtUiTest::Object *qo = qtuitest_cast<QtUiTest::Object*>(obj);
        if (!qo) {
            continue;
        }

        QVariant value = qo->getProperty(property);
        if (value.isValid()) {
            if (QObject *obj = value.value<QObject*>()) {
                value = QTestWidgets::signature(obj);
            }

            // Allow enums to match by value or by string
            if (value.type() == QVariant::Map
                && value.toMap().contains(QLatin1String("_value_"))) {
                if (searchValue.type() == QVariant::String
                    || searchValue.type() == QVariant::ByteArray) {
                    value = value.toMap().value(QLatin1String("_enum_"));
                } else {
                    value = value.toMap().value(QLatin1String("_value_"));
                }
            }

            if (matcher.match(value)) {
                foundList << obj;
            }
            continue;
        }

        if (qo->matchProperty(propertyUpper, searchValue)) {
            foundList << obj;
            continue;
        }
    }

    return foundList;
}

QObjectList QActiveTestWidget::queryProperties( const QVariantMap &searchValues, QtUiTest::QueryFlags flags )
{
    QTT_TRACE_FUNCTION();
    QSet<QObject*> initObjects;
    QObjectList searchList;

    if (!((flags & QtUiTest::NoScan) || (flags & QtUiTest::IgnoreScanned))) {
        if (!rescan(flags)) {
            return searchList;
        }
    }

    if (!(flags & QtUiTest::IgnoreScanned)) {
        initObjects << QTestWidgets::activeWidget();
        initObjects.unite(d->m_scannedItems);
    }

    static QStringList filterProperties(QStringList()
        << ancestorOfProperty << childOfProperty << descendantOfProperty
        << parentOfProperty << resourceOfProperty);

    QVariantMap::const_iterator i = searchValues.constBegin();
    while (i != searchValues.constEnd()) {
        QString property = i.key().toUpper();
        if (filterProperties.contains(property)) {
            QObjectList filterList;
            QString signature;
            if (i.value().type() == QVariant::Map) {
                signature = i.value().toMap().value(QLatin1String("_signature_")).toString();
            } else {
                signature = i.value().toString();
            }

            QString error;
            QObject *w;

            if (findWidget(signature, w, error)) {
                QtUiTest::Object *qw = qtuitest_cast<QtUiTest::Object*>(w);
                if (!qw) {
                    QtUiTest::setErrorString(QString("Can't find widget '%1' for property '%2'").arg(signature).arg(property));
                    return searchList;
                }
                if (property == childOfProperty) {
                    filterList << qw->children();
                } else if (property == descendantOfProperty) {
                    filterList << qw->descendants();
                } else if (property == ancestorOfProperty) {
                    filterList << qw->ancestors();
                } else if (property == parentOfProperty) {
                    filterList << qw->parent();
                } else if (property == resourceOfProperty) {
                    QVariantList resources = qw->resources();
                    foreach (const QVariant resource, resources) {
                        if (QObject *obj = resource.value<QObject*>()) {
                            filterList << obj;
                        }
                    }
                }
            }

            if (initObjects.isEmpty()) {
                initObjects = QSet<QObject*>::fromList(filterList);
            } else {
                foreach (QObject *obj, initObjects) {
                    if (!filterList.contains(obj)) {
                        initObjects.remove(obj);
                    }
                }
                if (initObjects.isEmpty()) {
                    return searchList;
                }
            }
        }
        ++i;
    }

    searchList = initObjects.toList();

    i = searchValues.constBegin();
    while (i != searchValues.constEnd()) {
        if (!filterProperties.contains(i.key().toUpper())) {
            searchList = findObjectsByProperty(i.key(), i.value(), searchList);
            if (searchList.isEmpty())
                break;
        }
        ++i;
    }

    return searchList;
}

PropertyMatcher::PropertyMatcher(const QVariant &searchValue, bool &ok)
    : m_matchFunction(0)
    , m_searchDouble(0)
{
    ok = true;
    if (searchValue.type() == QVariant::Map) {
        QVariantMap map = searchValue.toMap();
        m_searchValue = map["_value_"];
        QString op = map["_op_"].toString();
        if (op == QLatin1String("eq")) {
            m_matchFunction = &PropertyMatcher::equals;
        } else if (op == QLatin1String("ne")) {
            m_matchFunction = &PropertyMatcher::notEquals;
        } else if (op == QLatin1String("in")) {
            m_matchFunction = &PropertyMatcher::containedIn;
            ok = m_searchValue.canConvert(QVariant::StringList);
            if (ok)
                m_searchList = m_searchValue.toStringList();
        } else if (op == QLatin1String("gt")) {
            m_matchFunction = &PropertyMatcher::greaterThan;
            ok = m_searchValue.canConvert(QVariant::Double);
            if (ok)
                m_searchDouble = m_searchValue.toDouble();
        } else if (op == QLatin1String("lt")) {
            m_matchFunction = &PropertyMatcher::lessThan;
            ok = m_searchValue.canConvert(QVariant::Double);
            if (ok)
                m_searchDouble = m_searchValue.toDouble();
        } else {
            ok = false;
        }
        return;
    } else if (searchValue.type() == QVariant::RegExp) {
        m_searchRegExp = searchValue.toRegExp();
        m_matchFunction = &PropertyMatcher::regExpMatch;
        ok = m_searchRegExp.isValid();
        return;
    }

    m_matchFunction = &PropertyMatcher::equals;
    m_searchValue = searchValue;
}

bool PropertyMatcher::match(const QVariant &value) const
{
    return (const_cast<const PropertyMatcher*>(this)->*m_matchFunction)(value);
}

bool PropertyMatcher::equals(const QVariant &value) const
{
    return value == m_searchValue;
}

bool PropertyMatcher::notEquals(const QVariant &value) const
{
    return value != m_searchValue;
}

bool PropertyMatcher::containedIn(const QVariant &value) const
{
    return m_searchList.contains(value.toString());
}

bool PropertyMatcher::greaterThan(const QVariant &value) const
{
    return value.toDouble() > m_searchDouble;
}

bool PropertyMatcher::lessThan(const QVariant &value) const
{
    return value.toDouble() < m_searchDouble;
}

bool PropertyMatcher::regExpMatch(const QVariant &value) const
{
    return m_searchRegExp.exactMatch(value.toString());
}
