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


#include "qtuitestwidgetinterface.h"
#include "qtuitestnamespace.h"

#include <QRect>
#include <QRegion>
#include <QVariant>

/*!
    \preliminary
    \class QtUiTest::WidgetFactory
    \inpublicgroup QtUiTestModule
    \brief The WidgetFactory class provides a factory interface
    for QtUiTest widget wrapper classes.

    QtUiTest::WidgetFactory is an abstract base class which enables the
    creation of QtUiTest wrapper objects around Qt widgets.

    Customizing QtUiTest behaviour for particular widgets is achieved by
    implementing one or more test widget classes which inherit from
    one or more QtUiTest widget interfaces,
    subclassing QtUiTest::WidgetFactory, reimplementing the pure virtual
    keys() and create() functions to create instances of the custom test
    widget classes, and exporting the factory class using the
    Q_EXPORT_PLUGIN2() macro.
*/

/*!
    \fn QtUiTest::WidgetFactory::create(QObject* object)

    Attempts to create a test widget to wrap \a object.  Returns the created
    test widget.  Returns 0 if this factory does not support wrapping
    \a object.

    The returned object is suitable for use with
    \l{QtUiTest}{qtuitest_cast}.

    This function will only be called for objects which inherit one of the
    classes returned by keys().
*/

/*!
    \fn QtUiTest::WidgetFactory::keys() const

    Returns the list of C++ class names this factory can generate test widgets
    for.

    Note that returning a class from this function does not guarantee that the
    factory will always be able to generate a test widget for that class.
*/

/*!
    \preliminary
    \class QtUiTest::Widget
    \inpublicgroup QtUiTestModule
    \brief The Widget class provides an abstract base class
    for all test widgets.

    QtUiTest::Widget encapsulates important information
    and functionality related to two-dimensional GUI elements.

    All test widgets should implement the QtUiTest::Widget interface,
    using multiple inheritance to implement other QtUiTest interfaces
    where suitable.
*/

/*!
    \fn const QRect& QtUiTest::Widget::geometry() const

    Returns the geometry of this widget in parent coordinates.
*/

/*!
    Returns the left of the widget, in global coordinates.

    \sa mapToGlobal()
*/
int QtUiTest::Widget::x() const
{
    return mapToGlobal(QPoint()).x();
}

/*!
    Returns the top of the widget, in global coordinates.

    \sa mapToGlobal()
*/
int QtUiTest::Widget::y() const
{
    return mapToGlobal(QPoint()).y();
}

/*!
    Returns the width of the widget.

    \sa geometry()
*/
int QtUiTest::Widget::width() const
{
    return geometry().width();
}

/*!
    Returns the height of the widget.

    \sa geometry()
*/
int QtUiTest::Widget::height() const
{
    return geometry().height();
}

/*!
    \fn bool QtUiTest::Widget::isVisible() const

    Returns true if this widget is currently visible.
*/

/*!
    \fn QObject* QtUiTest::Widget::parent() const

    Returns the parent of this widget, or 0 if this widget has no parent.

    The returned object may be an actual widget, or may be a wrapping
    test widget.  Therefore, the only safe way to use the returned value
    of this function is to cast it to the desired QtUiTest interface
    using \l{QtUiTest}{qtuitest_cast}.

    \sa QObject::parent(), children()
*/

/*!
    \fn const QObjectList& QtUiTest::Widget::children() const

    Returns all children of this widget.

    The returned objects may be actual widget instances, or may be wrapping
    test widgets.  Therefore, the only safe way to use the returned objects
    are to cast them to the desired QtUiTest interface using
    \l{QtUiTest}{qtuitest_cast}.

    Reimplementing this function allows widgets which are conceptually
    widgets but are not QObject subclasses to be wrapped.  This can be
    achieved by returning a list of test widgets which do not necessarily
    have underlying widget instances.

    \sa QObject::children(), parent()
*/

/*!
    \internal

    Returns all ancestors of this widget.

    \sa children(), parent()
*/
QObjectList QtUiTest::Object::ancestors() const
{
    QObjectList found;
    QObject *p = parent();
    if (p) {
        QtUiTest::Object* qw = qtuitest_cast<QtUiTest::Object*>(p);
        found << p << qw->ancestors();
    }
    return found;
}

/*!
    \internal

    Returns all descendants of this widget. The results are appended to \a descendants.

    \sa children(), parent()
*/
void QtUiTest::Object::descendants(QSet<QObject*> &descendants) const
{
    foreach (QObject *child, children()) {
        if (!descendants.contains(child)) {
            descendants.insert(child);
            QtUiTest::Object* qw = qtuitest_cast<QtUiTest::Object*>(child);
            if (qw)
                qw->descendants(descendants);
        }
    }
}

/*!
    \internal

    Convenience function to return descendants as QObjectList.

    \sa ancestors()
*/
QObjectList QtUiTest::Object::descendants() const
{
    QSet<QObject*> found;
    descendants(found);
    return QObjectList::fromSet(found);
}

/*!
    \fn QPoint QtUiTest::Widget::mapToGlobal(const QPoint& pos) const

    Maps \a pos from widget coordinates to global screen coordinates and
    returns the result.
*/

/*!
    \fn QPoint QtUiTest::Widget::mapFromGlobal(const QPoint& pos) const

    Maps \a pos from global screen coordinates to widget coordinates and
    returns the result.
*/

/*!
    Returns the center point of the widget. The base implementation
    returns geometry().center().

    \sa QtUiTest::Widget::geometry()
*/
QPoint QtUiTest::Widget::center() const
{
    QPoint ret;
    ret = mapToGlobal( geometry().center() );
    return ret;
}

/*!
    \fn bool QtUiTest::Widget::hasFocus() const
    Returns true if this widget currently has keyboard focus.
*/

/*!
    Returns the focus proxy of this widget, or 0 if this widget has no focus proxy.
    A widget may "have focus", but have a child widget that actually handles the
    focus.

    The returned object may be an actual widget, or may be a wrapping
    test widget.  Therefore, the only safe way to use the returned value
    of this function is to cast it to the desired QtUiTest interface
    using \l{QtUiTest}{qtuitest_cast}.

*/
QObject* QtUiTest::Widget::focusProxy() const
{
    return 0;
}

/*!
    Sets \a img to image of the widget. Returns true on success.

    The base implementation always returns false.
*/
bool QtUiTest::Widget::grabImage(QImage &img) const
{ Q_UNUSED(img); return false; }

/*!
    Returns true if this widget should be ignored by QtUiTest. If
    a widget is ignored, any child widgets will still be processed.

    The base implementation always returns false.

    \sa {Widget Specifiers}, {Querying Objects}
*/
bool QtUiTest::Widget::ignoreScan() const
{ return false; }

/*!
    Returns the widget's window.
*/
QObject* QtUiTest::Widget::window() const
{
    return 0;
}
/*!
    Returns the value of this widget's property specified by \a property.

    \sa setProperty()
*/
QVariant QtUiTest::Object::getProperty(const QString& property) const
{
    Q_UNUSED(property)
    return QVariant();
}

/*!
    Sets the value of this widget's property specified by \a property to \a value.
    Returns true on success, and false if the property could not be set.

    \sa getProperty()
*/
bool QtUiTest::Object::setProperty(const QString &property, const QVariant &value)
{
    Q_UNUSED(property)
    Q_UNUSED(value)
    return false;
}

/*!
    Return true if this widget matches the \a value of pseudo-property \a property.

    \sa getProperty()
*/
bool QtUiTest::Object::matchProperty(const QString &property, const QVariant &value) const
{
    Q_UNUSED(property)
    Q_UNUSED(value)
    return false;
}

/*!
    Returns a map containing all the widget's properties.

    \sa getProperty()
*/
QVariantMap QtUiTest::Object::getProperties() const
{
    return QVariantMap();
}

QVariantList QtUiTest::Object::resources() const
{
    QVariantList ret;
    return ret;
}

/*!
    Returns the type of widget.
*/
QString QtUiTest::Object::typeOf() const
{
    return QString();
}

/*!
    Returns the widget's list of superclasses.
*/
QStringList QtUiTest::Object::inheritance() const
{
    return QStringList();
}

/*!
    Returns true if the item has displayable contents.
*/
bool QtUiTest::Object::hasContents() const
{
    return false;
}

/*!
    Returns a string value for debugging purposes.
*/
QString QtUiTest::Object::debugString() const
{
    return QString();
}
