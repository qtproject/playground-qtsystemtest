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


#include "qtuitestnamespace.h"

#include "qeventwatcher_p.h"
#include "qtuitestconnectionmanager_p.h"
#include "qtuitestwidgets_p.h"

#include <QtSystemTest/QstDebug>
#include <QEventLoop>
#include <QKeyEvent>
#include <QKeySequence>
#include <QPointer>
#include <QTimer>

/*
    A simple auto pointer class which deletes the pointed-to QObject
    later via deleteLater()
*/
template <typename T>
class QDelayedAutoPointer
{
public:
    inline QDelayedAutoPointer(T* thing)
        : raw(thing)
    {}
    inline ~QDelayedAutoPointer()
    {
        QObject::disconnect(raw, 0, 0, 0);
        raw->deleteLater();
    }

    inline T* operator->()
    { return raw; }

    inline operator T*()
    { return raw; }

private:
    T* raw;
};

/*
    A helper class to encapsulate generation of input events
*/
struct QtUiTestInput
{
    virtual ~QtUiTestInput() {}
    virtual void post() const =0;
    virtual QString toString() const =0;
};


namespace QtUiTest {
    QString toString(QObject* obj)
    {
        if (!obj) return "QObject(0x0)";
        return QString("%1(0x%2 \"%3\")")
            .arg(obj->metaObject()->className())
            .arg(QString::number(qptrdiff(obj), 16))
            .arg(obj->objectName())
        ;
    }

    QString toString(QEvent::Type type)
    {
#define DO(A) if (type == QEvent::A) return #A
        DO(KeyPress);
        DO(KeyRelease);
        DO(MouseButtonPress);
        DO(MouseButtonRelease);
        DO(Show);
        DO(Hide);
        DO(FocusIn);
        DO(FocusOut);
        DO(WindowBlocked);
#undef  DO
        return QString::number(int(type));
    }

    QString toString(QList<QEvent::Type> const& types)
    {
        QString ret;
        QString sep;
        foreach (QEvent::Type type, types) {
            ret += sep + toString(type);
            sep = ",";
        }
        return ret;
    }

    QString toString(Qt::Key key)
    { return QKeySequence(key).toString(); }

    QString toString(const QPoint& pos)
    { return QString("(%1,%2)").arg(pos.x()).arg(pos.y()); }

    QString toString(Qt::MouseButtons const& buttons)
    {
        QStringList ret;
#define DO(A) if (buttons & Qt::A) ret << #A
        DO(LeftButton);
        DO(RightButton);
        DO(MidButton);
        DO(XButton1);
        DO(XButton2);
#undef  DO
        return ret.join(",");
    }

    bool keyClick(QObject*, QList<QEvent::Type> const&, int, Qt::KeyboardModifiers, QtUiTest::InputOption);
    bool mouseClick(QObject*, QList<QEvent::Type> const&, const QPoint&, Qt::MouseButtons, QtUiTest::InputOption);
    bool mouseDClick(QObject*, QList<QEvent::Type> const&, const QPoint&, Qt::MouseButtons, QtUiTest::InputOption);
    bool inputWithEvent(QObject*, QList<QEventWatcherFilter*> const&, QtUiTestInput const&);
    bool inputWithSignal(QObject*, const QByteArray&, QtUiTestInput const&);
};

struct QtUiTestKeyClick : public QtUiTestInput
{
    QtUiTestKeyClick(Qt::Key key, Qt::KeyboardModifiers modifiers, QtUiTest::InputOption options)
        : m_key(key), m_modifiers(modifiers), m_options(options)
    {}

    virtual void post() const
    { QtUiTest::keyClick(m_key, m_modifiers, m_options); }

    virtual QString toString() const
    { return QString("Key click \"%1\"").arg(QtUiTest::toString(m_key)); }

    Qt::Key                m_key;
    Qt::KeyboardModifiers  m_modifiers;
    QtUiTest::InputOption  m_options;
};

struct QtUiTestMouseClick : public QtUiTestInput
{
    QtUiTestMouseClick(const QPoint& pos, Qt::MouseButtons buttons, QtUiTest::InputOption options)
        : m_pos(pos), m_buttons(buttons), m_options(options)
    {}

    virtual void post() const
    { QtUiTest::mouseClick(m_pos, m_buttons, m_options); }

    virtual QString toString() const
    {
        return QString("Mouse click \"%1\" at %2")
            .arg(QtUiTest::toString(m_buttons))
            .arg(QtUiTest::toString(m_pos))
        ;
    }

    const QPoint&          m_pos;
    Qt::MouseButtons       m_buttons;
    QtUiTest::InputOption  m_options;
};

struct QtUiTestMouseDClick : public QtUiTestInput
{
    QtUiTestMouseDClick(const QPoint& pos, Qt::MouseButtons buttons, QtUiTest::InputOption options)
        : m_pos(pos), m_buttons(buttons), m_options(options)
    {}

    virtual void post() const
    { QtUiTest::mouseDClick(m_pos, m_buttons, m_options); }

    virtual QString toString() const
    {
        return QString("Mouse click \"%1\" at %2")
            .arg(QtUiTest::toString(m_buttons))
            .arg(QtUiTest::toString(m_pos))
        ;
    }

    const QPoint&          m_pos;
    Qt::MouseButtons       m_buttons;
    QtUiTest::InputOption  m_options;
};

/*
    Filter which implements watching for events of a specific type.
*/
class QEventWatcherTypeFilter : public QEventWatcherFilter
{
public:
    QEventWatcherTypeFilter(QEvent::Type type)
        : m_type(type)
    {}

protected:
    virtual bool accept(QObject*,QEvent* e) const
    { return e->type() == m_type; }

    virtual QString toString() const
    { return QString("event of type %1").arg(QtUiTest::toString(m_type)); }

private:
    QEvent::Type m_type;
};

/*
    Filter which implements watching for specific key events.
*/

class QEventWatcherKeyFilter : public QEventWatcherTypeFilter
{
public:
    QEventWatcherKeyFilter(Qt::Key key, QEvent::Type type)
        : QEventWatcherTypeFilter(type)
        , m_key(key)
    {}

    static QEvent::Type keyPressType()
    { static int ret = QEvent::registerEventType(); return QEvent::Type(ret); }

    static QEvent::Type keyReleaseType()
    { static int ret = QEvent::registerEventType(); return QEvent::Type(ret); }

protected:
    virtual bool accept(QObject* o, QEvent* e) const
    {
        if (!QEventWatcherTypeFilter::accept(o,e))
            return false;
        if (e->type() != QEvent::KeyPress && e->type() != QEvent::KeyRelease)
            return false;
        return static_cast<QKeyEvent*>(e)->key() == m_key;
    }

    virtual QString toString() const
    {
        return QString("%1 (key:%2)")
            .arg(QEventWatcherTypeFilter::toString())
            .arg(QtUiTest::toString(m_key));
    }

private:
    Qt::Key m_key;
};


/*!
    \preliminary
    \namespace QtUiTest
    \inpublicgroup QtUiTestModule

    \brief The QtUiTest namespace provides the plugin interfaces used for
    customizing the behaviour of QtUiTest.
*/

/*!
    \fn T QtUiTest::qtuitest_cast_helper(QObject* object,T dummy)
    \internal
*/

/*!
    \relates QtUiTest
    \fn T qtuitest_cast(const QObject *object)

    Casts \a object to the specified QtUiTest test widget interface \c{T}.

    If \a object already implements \c{T}, it is simply casted and returned.
    Otherwise, QtUiTest will attempt to find or create a test widget to
    wrap \a object, using all loaded QtUiTest::WidgetFactory plugins.
    If a test widget cannot be created to wrap \a object, 0 is returned.

    In either case, the returned value must not be deleted by the caller.
*/


/*!
    \enum QtUiTest::InputOption

    This enum type specifies the options to be used when simulating key
    and mouse events.

    \value NoOptions no options.
    \value KeyRepeat when simulating key press events, simulate auto-repeat
                     key press events. The default is to simulate regular key
                     press events.
*/

/*!
    \enum QtUiTest::Key

    This enum provides mappings for high-level conceptual keys to platform-specific
    values of Qt::Key.

    \value Key_Activate         Key used to activate generic UI elements.
    \value Key_ActivateButton   Key used to activate buttons.
    \value Key_Select           Key used to select an item from lists.
*/

/*!
    Set or clear the specified \a option for subsequent simulated input
    events.  The option is set if \a on is true, otherwise it is cleared.
*/
void QtUiTest::setInputOption(QtUiTest::InputOption option, bool on)
{
    QTT_TRACE_FUNCTION();
    QtUiTestWidgets::instance()->setInputOption(option, on);
}

/*!
    Returns true if \a option is currently set.
*/
bool QtUiTest::testInputOption(QtUiTest::InputOption option)
{
    QTT_TRACE_FUNCTION();
    return QtUiTestWidgets::instance()->testInputOption(option);
}

/*!
    Simulate a mouse press event at the co-ordinates given by \a pos,
    for the specified \a buttons.  \a options are applied to the simulated
    event.

    \a pos is interpreted as local co-ordinates for the currently active
    window in this application.
*/
void QtUiTest::mousePress(const QPoint& pos, Qt::MouseButtons buttons,
            QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    QtUiTestWidgets::instance()->mousePress(pos, buttons, options);
}

void QtUiTest::touchPress(const QVariantList &points)
{
    QTT_TRACE_FUNCTION();
    QtUiTestWidgets::instance()->touchPress(points);
}

void QtUiTest::touchUpdate(const QVariantList &points)
{
    QTT_TRACE_FUNCTION();
    QtUiTestWidgets::instance()->touchUpdate(points);
}

void QtUiTest::touchRelease(const QVariantList &points)
{
    QTT_TRACE_FUNCTION();
    QtUiTestWidgets::instance()->touchRelease(points);
}

/*!
    Simulate a mouse release event at the global co-ordinates given by \a pos,
    for the specified \a buttons.  \a options are applied to the simulated
    event.

    \a pos is interpreted as local co-ordinates for the currently active
    window in this application.
*/
void QtUiTest::mouseRelease(const QPoint& pos, Qt::MouseButtons buttons,
            QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    QtUiTestWidgets::instance()->mouseRelease(pos, buttons, options);
}

/*!
    Simulate a mouse click event at the global co-ordinates given by \a pos,
    for the specified \a buttons.  \a options are applied to the simulated
    event.

    \a pos is interpreted as local co-ordinates for the currently active
    window in this application.
*/
void QtUiTest::mouseClick(const QPoint& pos, Qt::MouseButtons buttons,
            QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    QtUiTestWidgets::instance()->mouseClick(pos, buttons, options);
}

/*!
    Simulate a mouse double-click event at the global co-ordinates given by \a pos,
    for the specified \a buttons.  \a options are applied to the simulated
    event.

    \a pos is interpreted as local co-ordinates for the currently active
    window in this application.
*/
void QtUiTest::mouseDClick(const QPoint& pos, Qt::MouseButtons buttons,
            QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    QtUiTestWidgets::instance()->mouseDClick(pos, buttons, options);
}

/*!
    \overload
    Simulate a mouse click event.
    Returns true if the event appears to be delivered to \a object within maximumUiTimeout()
    milliseconds.
    If it does not, the errorString() will be set accordingly.
*/
bool QtUiTest::mouseClick(QObject* object, const QPoint& pos, Qt::MouseButtons buttons,
        QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    return mouseClick(object,
        QList<QEvent::Type>() << QEvent::MouseButtonRelease << QEvent::Hide << QEvent::WindowBlocked,
        pos, buttons, options
    );
}

/*!
    \overload
    Simulate a mouse double-click event.
    Returns true if the event appears to be delivered to \a object within maximumUiTimeout()
    milliseconds.
    If it does not, the errorString() will be set accordingly.
*/
bool QtUiTest::mouseDClick(QObject* object, const QPoint& pos, Qt::MouseButtons buttons,
        QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    return mouseDClick(object,
        QList<QEvent::Type>() << QEvent::MouseButtonRelease << QEvent::Hide << QEvent::WindowBlocked,
        pos, buttons, options
    );
}

/*!
    Simulate moving the mouse pointer to the global co-ordinates given by \a pos.
    \a options are applied to the simulated event.
*/
void QtUiTest::mouseMove(const QPoint& pos, QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    QtUiTestWidgets::instance()->mouseMove(pos, options);
}

/*!
    \internal
*/
bool QtUiTest::mouseClick(QObject* object, QList<QEvent::Type> const& types, const QPoint& pos,
        Qt::MouseButtons buttons, QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    QList<QEventWatcherFilter*> filters;
    foreach (QEvent::Type type, types) {
        filters << new QEventWatcherTypeFilter(type);
    }
    return inputWithEvent(object, filters, QtUiTestMouseClick(pos, buttons, options));
}

/*!
    \internal
*/
bool QtUiTest::mouseDClick(QObject* object, QList<QEvent::Type> const& types, const QPoint& pos,
        Qt::MouseButtons buttons, QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    QList<QEventWatcherFilter*> filters;
    foreach (QEvent::Type type, types) {
        filters << new QEventWatcherTypeFilter(type);
    }
    return inputWithEvent(object, filters, QtUiTestMouseDClick(pos, buttons, options));
}

/*!
    \overload
    Simulate a mouse click event.
    Returns true if the event causes \a object to emit \a signal within maximumUiTimeout()
    milliseconds.
    If it does not, the errorString() will be set accordingly.
*/
bool QtUiTest::mouseClick(QObject* object, const QByteArray& signal, const QPoint& pos,
        Qt::MouseButtons buttons, QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    return inputWithSignal(object, signal, QtUiTestMouseClick(pos, buttons, options));
}

/*!
    \overload
    Simulate a mouse double-click event.
    Returns true if the event causes \a object to emit \a signal within maximumUiTimeout()
    milliseconds.
    If it does not, the errorString() will be set accordingly.
*/
bool QtUiTest::mouseDClick(QObject* object, const QByteArray& signal, const QPoint& pos,
        Qt::MouseButtons buttons, QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    return inputWithSignal(object, signal, QtUiTestMouseDClick(pos, buttons, options));
}

/*!
    Simulate a key press event, using the given \a key and \a modifiers.
    \a key must be a valid Qt::Key or QtUiTest::Key.
    \a options are applied to the simulated event.
*/
void QtUiTest::keyPress(int key, Qt::KeyboardModifiers modifiers,
        QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    QtUiTestWidgets::instance()->keyPress(static_cast<Qt::Key>(key), modifiers, options);
}

/*!
    Simulate a key release event, using the given \a key and \a modifiers.
    \a key must be a valid Qt::Key or QtUiTest::Key.
    \a options are applied to the simulated event.
*/
void QtUiTest::keyRelease(int key, Qt::KeyboardModifiers modifiers,
        QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    QtUiTestWidgets::instance()->keyRelease(static_cast<Qt::Key>(key), modifiers, options);
}

/*!
    Simulate a key click event, using the given \a key and \a modifiers.
    \a key must be a valid Qt::Key or QtUiTest::Key.
    \a options are applied to the simulated event.
*/
void QtUiTest::keyClick(int key, Qt::KeyboardModifiers modifiers,
        QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    QtUiTestWidgets::instance()->keyClick(static_cast<Qt::Key>(key), modifiers, options);
}

/*!
    \overload
    Simulate a key click event.
    Returns true if the event appears to be delivered to \a object within maximumUiTimeout()
    milliseconds.
    If it does not, the errorString() will be set accordingly.
*/
bool QtUiTest::keyClick(QObject* object, int key, Qt::KeyboardModifiers modifiers,
        QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    return keyClick(object,
        QList<QEvent::Type>() << QEventWatcherKeyFilter::keyReleaseType()
            << QEvent::Hide << QEvent::WindowBlocked << QEvent::Show,
        key, modifiers, options
    );
}

/*!
    \internal
*/
bool QtUiTest::inputWithEvent(QObject* object, QList<QEventWatcherFilter*> const& filters,
        QtUiTestInput const& event)
{
    QTT_TRACE_FUNCTION();
    QPointer<QObject> sender = object;
    QDelayedAutoPointer<QEventWatcher> w = new QEventWatcher;
    w->addObject(sender);
    foreach (QEventWatcherFilter* filter, filters)
        w->addFilter(filter);

    event.post();

    if (!w->count() && !QtUiTest::waitForSignal(w, SIGNAL(event(QObject*,int)))) {
        setErrorString(QString(
            "%1 was expected to result in %2 receiving an event matching one of the following, "
            "but it didn't:\n%3")
            .arg(event.toString())
            .arg(toString(sender))
            .arg(w->toString()));
        return false;
    }
    return true;
}

/*!
    \internal
*/
bool QtUiTest::inputWithSignal(QObject* object, const QByteArray& signal,
        QtUiTestInput const& event)
{
    QTT_TRACE_FUNCTION();
    if (signal.isEmpty()) return false;
    QPointer<QObject> sender = object;

    QTimer dummy;
    dummy.setInterval(1000);
    if (!QtUiTest::connectFirst(sender, signal, &dummy, SLOT(start()))) {
        setErrorString(QString("Object %1 has no signal %2").arg(toString(sender)).arg(&signal.constData()[1]));
        return false;
    }

    // Ensure connectNotify is called
    if (!QObject::connect(sender, signal, &dummy, SLOT(start())))
        Q_ASSERT(0);

    event.post();

    if (!dummy.isActive() && !QtUiTest::waitForSignal(sender, signal)) {
        setErrorString(QString(
            "%1 was expected to result in %2 emitting the signal %3, "
            "but it didn't.")
            .arg(event.toString())
            .arg(toString(sender))
            .arg(&signal.constData()[1]));
        return false;
    }
    return true;
}

/*!
    \internal
    \overload
    Simulate a key click event.
    Returns true if \a object receives any event of the given \a types within maximumUiTimeout()
    milliseconds.
    If it does not, the errorString() will be set accordingly.
*/
bool QtUiTest::keyClick(QObject* object, QList<QEvent::Type> const& types, int key,
        Qt::KeyboardModifiers modifiers, QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    QList<QEventWatcherFilter*> filters;
    foreach (QEvent::Type type, types) {
        // These cases result in waiting for specific key events rather than just "any key press".
        if (type == QEventWatcherKeyFilter::keyPressType()) {
            filters << new QEventWatcherKeyFilter(Qt::Key(key), QEvent::KeyPress);
        } else if (type == QEventWatcherKeyFilter::keyReleaseType()) {
            filters << new QEventWatcherKeyFilter(Qt::Key(key), QEvent::KeyRelease);
        } else {
            filters << new QEventWatcherTypeFilter(type);
        }
    }
    return inputWithEvent(object, filters, QtUiTestKeyClick(Qt::Key(key), modifiers, options));
}

/*!
    \overload
    Simulate a key click event.
    Returns true if \a object emits \a signal within maximumUiTimeout()
    milliseconds.
    If it does not, the errorString() will be set accordingly.
*/
bool QtUiTest::keyClick(QObject* object, const QByteArray& signal, int key, Qt::KeyboardModifiers modifiers,
        QtUiTest::InputOption options)
{
    QTT_TRACE_FUNCTION();
    return inputWithSignal(object, signal, QtUiTestKeyClick(Qt::Key(key), modifiers, options));
}

/*!
    Returns the maximum amount of time, in milliseconds, the user interface is allowed to take
    to generate some response to a user's action.

    This value is useful to determine how long test widgets should wait for certain events to occur
    after simulating key/mouse events.  The value may be device-specific.
*/
int QtUiTest::maximumUiTimeout()
{ return 2000; }

/*!
    Returns the Qt::Key corresponding to \a c.

    This function is commonly used in conjunction with keyClick() to enter
    a string of characters using the keypad.

    Example:
    \code
    using namespace QtUiTest;
    QString text = "hello world";
    // ...
    foreach (QChar c, text) {
        keyClick( asciiToKey(c.toLatin1()), asciiToModifiers(c.toLatin1()) );
    }
    \endcode
*/
// Now implemented in qasciikey.cpp

/*!
    Returns any Qt::KeyboardModifiers which would be required to input \a c.

    This function is commonly used in conjunction with keyClick() to enter
    a string of characters using the keypad.

    Example:
    \code
    using namespace QtUiTest;
    QString text = "hello world";
    // ...
    foreach (QChar c, text) {
        keyClick( asciiToKey(c.toLatin1()), asciiToModifiers(c.toLatin1()) );
    }
    \endcode
*/
Qt::KeyboardModifiers QtUiTest::asciiToModifiers(char c)
{
    QTT_TRACE_FUNCTION();
    Qt::KeyboardModifiers ret = Qt::NoModifier;
    if (QChar(c).isUpper()) ret |= Qt::ShiftModifier;
    return ret;
}

/*!
    Returns a human-readable error string describing the last error which
    occurred while accessing a testwidget.

    The error string is used to report directly to a tester any unexpected
    errors.  The string will typically be used as a test failure message.

    \sa setErrorString()
*/
QString QtUiTest::errorString()
{
    return QtUiTestWidgets::instance()->errorString();
}

/*!
    Returns a QDebug object which can be used as an output stream for debugging
    information. The debug information is sent to the test runner for output.
    This ensures a consistent handling of debugging information on different
    test platforms.

    Example:
    \code
    QtUiTest::debug() << "This is a debug message";
    \endcode
*/
QDebug QtUiTest::debug()
{
    return QtUiTestWidgets::instance()->debug();
}

/*!
    \internal
*/
QString QtUiTest::debugOutput()
{
    return QtUiTestWidgets::instance()->debugOutput();
}

/*!
    Sets the human-readable \a error string describing the last error which
    occurred while accessing a testwidget.

    \sa errorString()
*/
void QtUiTest::setErrorString(const QString& error)
{
    QtUiTestWidgets::instance()->setErrorString(error);
}

/*!
    \internal
    Returns a test widget wrapper for \a object which implements
    \a interface.
*/
QObject* QtUiTest::testWidget(QObject* object, const char* interface)
{
    return QtUiTestWidgets::instance()->testWidget(object, interface);
}

/*!
    Causes the process to wait for \a ms milliseconds. While waiting, events will be processed.
*/
void QtUiTest::wait(int ms)
{
    QTT_TRACE_FUNCTION();
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, SLOT(quit()));
    loop.exec();
}

#include <QDebug>

/*!
    Causes the process to wait for \a ms milliseconds or until \a signal is
    emitted from \a object, whichever comes first.

    While waiting, events will be processed.

    Returns true if \a signal was emitted from \a object before timing out.
    When false is returned, errorString() is set accordingly.

    If \a connectionType specifies a direct connection, this function will return
    immediately when the signal occurs, possibly before some objects have
    received the signal.  If \a connectionType specifies a queued connection, this
    function will return once the event loop has run following the emit.
*/
bool QtUiTest::waitForSignal(QObject* object, const char* signal, int ms, Qt::ConnectionType connectionType)
{
    QTT_TRACE_FUNCTION();
    if (ms < 0) return false;
    if (!signal || !signal[0]) return false;

    QPointer<QObject> sender = object;

    // Dummy variable to detect signal emission.
    QTimer dummy;
    dummy.setInterval(1000);
    if (!QtUiTest::connectFirst(sender, signal, &dummy, SLOT(start())))
        return false;

    // Ensure connectNotify is called
    if (!QObject::connect(sender, signal, &dummy, SLOT(start())))
        Q_ASSERT(0);

    QEventLoop loop;
    if (!QObject::connect(sender, signal, &loop, SLOT(quit())))
        return false;
    QTimer::singleShot(ms, &loop, SLOT(quit()));

    loop.exec();

    return dummy.isActive();
}

/*!
    Causes the process to wait for \a ms milliseconds or until an event of
    any of the given \a types is received by \a object, whichever comes first.

    While waiting, events will be processed.

    Returns true if the event was received by \a object before timing out.
    When false is returned, errorString() is set accordingly.

    If \a connectionType specifies a direct connection, this function will return
    immediately before the event is processed by \a object.
    If \a connectionType specifies a queued connection, this function will return
    once the event loop has run following the processing of the event.
*/
bool QtUiTest::waitForEvent(QObject* object, QList<QEvent::Type> const& types, int ms, Qt::ConnectionType connectionType)
{
    QTT_TRACE_FUNCTION();
    QPointer<QObject> sender = object;
    QDelayedAutoPointer<QEventWatcher> w = new QEventWatcher;
    w->setObjectName("qtuitest_waitForEvent_watcher");
    w->addObject(sender);
    foreach (QEvent::Type type, types)
        w->addFilter(new QEventWatcherTypeFilter(type));

    if (!QtUiTest::waitForSignal(w, SIGNAL(event(QObject*,int)), ms, connectionType)) {
        setErrorString(QString("Object %1 was expected to receive an event of type(s) %2 within "
            "%3 milliseconds, but it didn't.")
            .arg(toString(sender))
            .arg(toString(types))
            .arg(ms)
        );
        return false;
    }
    return true;
}

/*!
    \overload
    Waits for an event of the given \a type.
*/
bool QtUiTest::waitForEvent(QObject* object, QEvent::Type type, int ms, Qt::ConnectionType connectionType)
{
    QTT_TRACE_FUNCTION();
    return waitForEvent(object, QList<QEvent::Type>() << type, ms, connectionType);
}

/*!
    Creates a connection from the \a signal in the \a sender object to
    the \a method in the \a receiver object. Returns true if the connection succeeds;
    otherwise returns false.

    This function behaves similarly to QObject::connect() with the following
    important differences.
    \list
        \o The connection is guaranteed to be activated before
           any connections made with QObject::connect().
        \o The connection type is always Qt::DirectConnection.
        \o The connection cannot be disconnected using QObject::disconnect()
           (QtUiTest::disconnectFirst() must be used instead).
        \o The connection does not affect the return value of QObject::receivers().
        \o While \a method is being executed, the return value of
           QObject::sender() is undefined.
        \o QObject::connectNotify() is not called on the sending object.
    \endlist

    This function is primarily used in conjunction with QtUiTestRecorder to
    ensure events are recorded in the correct order.

    Note that this function cannot be used in a program which uses QSignalSpy.

    \sa QObject::connect(), QtUiTest::disconnectFirst()
*/
bool QtUiTest::connectFirst(const QObject* sender,   const char* signal,
                            const QObject* receiver, const char* method)
{
    QTT_TRACE_FUNCTION();
    // On failure, we use QObject::connect to get the same error message as
    // we normally would.
    if (sender == 0 || receiver == 0 || signal == 0 || method == 0) {
        return QObject::connect(sender,signal,receiver,method);
    }
    if (qstrlen(signal) < 1 || qstrlen(method) < 1) {
        return QObject::connect(sender,signal,receiver,method);
    }

    const QMetaObject* const senderMo = sender->metaObject();

    QByteArray normalSignal = QByteArray::fromRawData(signal+1, qstrlen(signal)-1);
    int signal_index = senderMo->indexOfSignal(normalSignal);

    if (signal_index < 0) {
        // See if we can find the signal after normalizing.
        normalSignal = QMetaObject::normalizedSignature(normalSignal);
        signal_index = senderMo->indexOfSignal(normalSignal);
    }
    if (signal_index < 0) {
        // Nope, bail out.
        return QObject::connect(sender,signal,receiver,method);
    }

    const QMetaObject* const receiverMo = receiver->metaObject();

    QByteArray normalMethod = QByteArray::fromRawData(method+1, qstrlen(method)-1);
    int method_index = receiverMo->indexOfMethod(normalMethod);

    if (method_index < 0) {
        // See if we can find the method after normalizing.
        normalMethod = QMetaObject::normalizedSignature(normalMethod);
        method_index = senderMo->indexOfMethod(normalMethod);
    }
    if (method_index < 0) {
        // Nope, bail out.
        return QObject::connect(sender,signal,receiver,method);
    }

    // Ensure signal and slot are compatible.
    if (!QMetaObject::checkConnectArgs(normalSignal.constData(), normalMethod.constData())) {
        return QObject::connect(sender,signal,receiver,method);
    }

    // If we get here, then everything is valid.
    QtUiTestConnectionManager::instance()->connect(sender, signal_index, receiver, method_index);
    return true;
}

/*!
    Disconnects \a signal in object \a sender from \a method in object \a receiver. Returns true if the connection is successfully broken; otherwise returns false.

    The connection must have been established with QtUiTest::connectFirst().

    Passing null arguments has the same wildcard effects as documented in QObject::disconnect().

    If the same connection has been established multiple times, disconnectFirst() will disconnect
    all instances of the connection.  There is no way to disconnect a single instance of a
    connection.  This behavior matches QObject::disconnect().

    \sa QObject::disconnect(), QtUiTest::connectFirst()
*/
bool QtUiTest::disconnectFirst(const QObject* sender,   const char* signal,
                               const QObject* receiver, const char* method)
{
    QTT_TRACE_FUNCTION();
    // On failure, we use QObject::disconnect to get the same error message as
    // we normally would.
    if (sender == 0) {
        return QObject::disconnect(sender,signal,receiver,method);
    }

    const QMetaObject* const senderMo = sender->metaObject();

    QByteArray normalSignal = (signal)
        ? QByteArray::fromRawData(signal+1, qstrlen(signal)-1)
        : QByteArray();
    int signal_index = (signal) ? senderMo->indexOfSignal(normalSignal) : -1;
    if (signal && (signal_index < 0)) {
        // See if we can find the signal after normalizing.
        normalSignal = QMetaObject::normalizedSignature(signal);
        signal_index = senderMo->indexOfSignal(normalSignal);
    }
    if (signal && (signal_index < 0)) {
        // Nope, bail out.
        return QObject::disconnect(sender,signal,receiver,method);
    }

    if (method && !receiver) {
        return QObject::disconnect(sender,signal,receiver,method);
    }

    const QMetaObject* const receiverMo = (receiver) ? receiver->metaObject() : 0;

    QByteArray normalMethod = (method)
        ? QByteArray::fromRawData(method+1, qstrlen(method)-1)
        : QByteArray();
    int method_index = (method) ? receiverMo->indexOfMethod(normalMethod) : -1;

    if (method && (method_index < 0)) {
        // See if we can find the method after normalizing.
        normalMethod = QMetaObject::normalizedSignature(method);
        method_index = senderMo->indexOfMethod(normalMethod);
    }
    if (method && (method_index < 0)) {
        // Nope, bail out.
        return QObject::disconnect(sender,signal,receiver,method);
    }

    return QtUiTestConnectionManager::instance()->disconnect(sender,signal_index,receiver,method_index);
}

