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


#include "qinputgenerator_p.h"
#include "qtuitestwidgetinterface.h"
#include "qtuitestwidgets_p.h"
#include "testquickfactory.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    #include "qinputgenerator_qwindow_p.h"
#else
    #include "qinputgenerator_generic_p.h"
#endif

#include <QtSystemTest/QstDebug>

/*
    If the QTUITEST_INPUT_DELAY environment variable is set, all calls to keyClick etc will
    immediately return but the resulting event won't be generated until QTUITEST_INPUT_DELAY
    milliseconds have passed.
    This can be used to simulate a slow system to catch race conditions.
    For example, setting QTUITEST_INPUT_DELAY to 500 roughly simulates running with a remote X
    server over a link with a round trip time of 250ms.
*/

#include <QDir>
#include <QPluginLoader>
#include <QPointer>
#include <QStringList>
#include <QTimer>
#include <QPoint>

/*!
    \internal
    \class QtUiTestWidgets
    \brief The QtUiTestWidgets class provides an interface for creating and managing QtUiTest widgets.

    QtUiTestWidgets manages the lifetime of all test widgets and allows
    test widgets to simulate user interaction with the application.
*/

enum QtUiTestInputType { QtUiTestPress, QtUiTestRelease, QtUiTestClick, QtUiTestDClick, QtUiTestMove };
struct QtUiTestKeyEvent
{
    QtUiTestInputType     type;
    Qt::Key               key;
    Qt::KeyboardModifiers mod;
    QtUiTest::InputOption opt;
};
struct QtUiTestMouseEvent
{
    QtUiTestInputType     type;
    QPoint                pos;
    Qt::MouseButtons      state;
    QtUiTest::InputOption opt;
};
struct QtUiTestTouchEvent
{
    QtUiTestInputType     type;
    QVariantList          points;
};
static int qtUiTestGetInputDelay()
{
    bool ok;
    QByteArray value(qgetenv("QTUITEST_INPUT_DELAY"));
    int ret = value.toInt(&ok);
    if (!ok || ret < 0) ret = 0;//-1;
    return ret;
}
static int qtUiTestInputDelay()
{ static int ret = qtUiTestGetInputDelay(); return ret; }


class QtUiTestWidgetsPrivate
{
public:
    QtUiTestWidgetsPrivate(QtUiTestWidgets* parent);

    static QList<QByteArray> allImplementedInterfaces(QObject *o);

    void _q_objectDestroyed();

    QString currentInputMethod() const;

    QtUiTestWidgets* q;

    QHash< QByteArray, QSet<QtUiTest::WidgetFactory*> >      factories;
    QHash< QObject*,   QHash<QByteArray, QPointer<QObject> > > testWidgets;
    QSet< QtUiTest::WidgetFactory* > factorySet;

    int inputOptions;

    QString errorString;
    QString debugBuffer;

    QInputGenerator *input;

    QList<QtUiTestKeyEvent>   pendingKeyEvents;
    QList<QtUiTestMouseEvent> pendingMouseEvents;
    QList<QtUiTestTouchEvent> pendingTouchEvents;
    void _q_postNextKeyEvent();
    void _q_postNextMouseEvent();
    void _q_postNextTouchEvent();
};

class QTWOptStack
{
public:
    QTWOptStack(QtUiTestWidgetsPrivate* obj,
            QtUiTest::InputOption opt)
        : d(obj), option(opt)
    {
        if (opt && (!(d->inputOptions & opt))) {
            d->inputOptions |= opt;
        } else {
            d = 0;
        }
    }

    ~QTWOptStack()
    {
        if (d) {
            d->inputOptions &= ~option;
        }
    }

    QtUiTestWidgetsPrivate *d;
    QtUiTest::InputOption option;
};

QtUiTestWidgetsPrivate::QtUiTestWidgetsPrivate(QtUiTestWidgets* parent)
    :   q(parent),input(0),
        inputOptions(QtUiTest::NoOptions)
{
    QTT_TRACE_FUNCTION();
}

QtUiTestWidgets::QtUiTestWidgets()
    : QObject(),
      d(new QtUiTestWidgetsPrivate(this))
{
    QTT_TRACE_FUNCTION();
    d->input = new QInputGeneratorGeneric();
    refreshPlugins();
}

QtUiTestWidgets::~QtUiTestWidgets()
{
    QTT_TRACE_FUNCTION();
    delete d->input;
    delete d;
    d = 0;
}

/*!
    Returns a static instance of QtUiTestWidgets.
*/
QtUiTestWidgets* QtUiTestWidgets::instance()
{
    static QtUiTestWidgets instance;
    return &instance;
}

/*!
    When an object is destroyed, deletes all test widgets pointing
    to that object.
*/
void QtUiTestWidgetsPrivate::_q_objectDestroyed()
{
    QTT_TRACE_FUNCTION();
    QHash< QByteArray, QPointer<QObject> > toDestroy
        = testWidgets.take(q->sender());

    foreach (QPointer<QObject> tw, toDestroy.values()) {
        if (tw) delete tw;
    }
}

/*!
    \internal
    Destroy all test widgets and unregister all factories.
    After calling this, refreshPlugins() must be called to be able to
    construct testwidgets from factories.

    For testing purposes only.
*/
void QtUiTestWidgets::clear()
{
    QTT_TRACE_FUNCTION();
    d->factories.clear();
    d->factorySet.clear();

    foreach (QObject *o, d->testWidgets.keys()) {
        foreach (QPointer<QObject> tw, d->testWidgets[o].values()) {
            if (tw && tw != o) delete tw;
        }
    }
    d->testWidgets.clear();
}

void QtUiTestWidgets::refreshPlugins()
{
    registerFactory(new TestQuickFactory());
}

/*!
    Registers \a factory as a factory class for constructing test widgets.

    It is not necessary to explicitly call this from QtUiTest widget plugins.
    This function should only be called if a QtUiTest::WidgetFactory has been
    created without using the standard plugin interface.
*/
void
QtUiTestWidgets::registerFactory(QtUiTest::WidgetFactory* factory)
{
    QTT_TRACE_FUNCTION();
    if (!factory) return;

    d->factorySet << factory;
    foreach(QString k, factory->keys()) {
        d->factories[k.toLatin1()].insert(factory);
    }
}

/*!
    Returns a human-readable error string describing the last error which
    occurred while accessing a testwidget.

    The error string is used to report directly to a tester any unexpected
    errors.  The string will typically be used as a test failure message.

    \sa setErrorString()
*/
QString QtUiTestWidgets::errorString() const
{ return d->errorString; }

/*!
    Sets the human-readable \a error string describing the last error which
    occurred while accessing a testwidget.

    \sa errorString()
*/
void QtUiTestWidgets::setErrorString(const QString& error)
{
    if (error == d->errorString) return;
    d->errorString = error;
}

/*!
    Returns a QDebug object which can be used as an output stream for debugging
    information. The debug information is sent to the test runner for output.
    This ensures a consistent handling of debugging information on different
    test platforms.
*/
QDebug QtUiTestWidgets::debug() const
{
    return QDebug(&d->debugBuffer) << '\n';
}

/*!
    \internal
*/
QString QtUiTestWidgets::debugOutput()
{
    QString ret = d->debugBuffer;
    d->debugBuffer.clear();
    return ret;
}

/*!
    Returns a list of all QtUiTest widget interfaces implemented by \a o .
*/
QList<QByteArray> QtUiTestWidgetsPrivate::allImplementedInterfaces(QObject *o)
{
    QTT_TRACE_FUNCTION();
    // FIXME this function should not have to be explicitly implemented.
    // Find some way to automatically handle all interfaces.
    QList<QByteArray> ret;
    if (qobject_cast<QtUiTest::Object*>(o))          ret << "Object";
    if (qobject_cast<QtUiTest::Widget*>(o))          ret << "Widget";
    return ret;
}

/*!
    Returns a test widget wrapper for \a object implementing the given
    \a interface.  If a test widget implementing \a interface is already
    wrapping \a object, that test widget will be returned.  Otherwise,
    a new test widget may be constructed using registered factories.

    Returns 0 if the given \a interface is not implemented on \a object
    or on any test widget which wraps \a object.

    The returned object should not be deleted by the caller.  QtUiTestWidgets
    retains ownership of the returned test widget and deletes it when \a object
    is destroyed.

    \sa registerFactory()
*/
QObject* QtUiTestWidgets::testWidget(QObject* object, const QByteArray &interface)
{
    if (!object) return 0;

    QMetaObject const *mo = object->metaObject();
    QObject *ret = d->testWidgets.value( object ).value( interface );

    bool watchingDestroyed = false;

    if (!ret) {
        QSet<QtUiTest::WidgetFactory*> usedFactories;
        while (mo) {
            foreach (QtUiTest::WidgetFactory *factory,
                    d->factories.value(mo->className()) - usedFactories) {

                QObject *testWidget = factory->create(object);
                usedFactories.insert(factory);
                if (testWidget) {
                    bool isValuable = false;
                    foreach (const QByteArray& thisIface,
                             d->allImplementedInterfaces(testWidget)) {
                        QHash<QByteArray, QPointer<QObject> > &subhash
                            = d->testWidgets[object];
                        if (!subhash[thisIface]) {
                            isValuable = true;
                            subhash.insert( thisIface, testWidget );
                            watchingDestroyed = watchingDestroyed
                                || connect(object, SIGNAL(destroyed()),
                                        this, SLOT(_q_objectDestroyed()));
                        }
                    }
                    if (!isValuable) {
                        delete testWidget;
                    }
                }
            }
            mo = mo->superClass();
        }
        ret = d->testWidgets.value( object ).value( interface );
    }
    return ret;
}

/*!
    Simulate a mouse press event at the global co-ordinates given by \a pos,
    for the buttons in \a state.  \a opt is applied to the simulated event.
*/
void QtUiTestWidgets::mousePress(const QPoint &pos, Qt::MouseButtons state,
        QtUiTest::InputOption opt)
{
    QTT_TRACE_FUNCTION();
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestMouseEvent event = {QtUiTestPress, pos, state, opt};
        d->pendingMouseEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextMouseEvent()));
        return;
    }

    QTWOptStack st(d, opt);
    d->input->mousePress(pos, state);
}

void QtUiTestWidgets::touchPress(const QVariantList &points)
{
    QTT_TRACE_FUNCTION();
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestTouchEvent event = {QtUiTestPress, points };
        d->pendingTouchEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextTouchEvent()));
        return;
    }

    d->input->touchPress(points);
}

void QtUiTestWidgets::touchUpdate(const QVariantList &points)
{
    QTT_TRACE_FUNCTION();
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestTouchEvent event = {QtUiTestMove, points};
        d->pendingTouchEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextTouchEvent()));
        return;
    }

    d->input->touchUpdate(points);
}

void QtUiTestWidgets::touchRelease(const QVariantList &points)
{
    QTT_TRACE_FUNCTION();
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestTouchEvent event = {QtUiTestRelease, points};
        d->pendingTouchEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextTouchEvent()));
        return;
    }

    d->input->touchRelease(points);
}

/*!
    Simulate a mouse release event at the global co-ordinates given by \a pos,
    for the buttons in \a state.  \a opt is applied to the simulated event.
*/
void QtUiTestWidgets::mouseRelease(const QPoint &pos, Qt::MouseButtons state,
        QtUiTest::InputOption opt)
{
    QTT_TRACE_FUNCTION();
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestMouseEvent event = {QtUiTestRelease, pos, state, opt};
        d->pendingMouseEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextMouseEvent()));
        return;
    }

    QTWOptStack st(d, opt);
    d->input->mouseRelease(pos, state);
}

/*!
    Simulate a mouse click event at the global co-ordinates given by \a pos,
    for the buttons in \a state.  \a opt is applied to the simulated event.
*/
void QtUiTestWidgets::mouseClick(const QPoint &pos, Qt::MouseButtons state,
        QtUiTest::InputOption opt)
{
    QTT_TRACE_FUNCTION();
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestMouseEvent event = {QtUiTestClick, pos, state, opt};
        d->pendingMouseEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextMouseEvent()));
        return;
    }

    QTWOptStack st(d, opt);
    d->input->mouseClick(pos, state);
}

/*!
    Simulate a mouse double-click event at the global co-ordinates given by \a pos,
    for the buttons in \a state.  \a opt is applied to the simulated event.
*/
void QtUiTestWidgets::mouseDClick(const QPoint &pos, Qt::MouseButtons state,
        QtUiTest::InputOption opt)
{
    QTT_TRACE_FUNCTION();
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestMouseEvent event = {QtUiTestDClick, pos, state, opt};
        d->pendingMouseEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextMouseEvent()));
        return;
    }

    QTWOptStack st(d, opt);
    d->input->mouseDClick(pos, state);
}

/*!
    Simulate moving the mouse pointer to global co-ordinates given by \a pos,
    \a opt is applied to the simulated event.
*/
void QtUiTestWidgets::mouseMove(const QPoint &pos, QtUiTest::InputOption opt)
{
    QTT_TRACE_FUNCTION();
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestMouseEvent event = {QtUiTestMove, pos, 0, opt};
        d->pendingMouseEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextMouseEvent()));
        return;
    }

    QTWOptStack st(d, opt);
    d->input->mouseMove(pos);
}

/*!
    Simulate a key press event, using the given \a key and \a mod.
    \a opt is applied to the simulated event.
*/
void QtUiTestWidgets::keyPress(Qt::Key key, Qt::KeyboardModifiers mod,
        QtUiTest::InputOption opt)
{
    QTT_TRACE_FUNCTION();
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestKeyEvent event = {QtUiTestPress, key, mod, opt};
        d->pendingKeyEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextKeyEvent()));
        return;
    }

    QTWOptStack st(d, opt);
    d->input->keyPress(key, mod, opt & QtUiTest::KeyRepeat);
}

/*!
    Simulate a key release event, using the given \a key and \a mod.
    \a opt is applied to the simulated event.
*/
void QtUiTestWidgets::keyRelease(Qt::Key key, Qt::KeyboardModifiers mod,
        QtUiTest::InputOption opt)
{
    QTT_TRACE_FUNCTION();
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestKeyEvent event = {QtUiTestRelease, key, mod, opt};
        d->pendingKeyEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextKeyEvent()));
        return;
    }

    QTWOptStack st(d, opt);
    d->input->keyRelease(key, mod);
}

/*!
    Simulate a key click event, using the given \a key and \a mod.
    \a opt is applied to the simulated event.
*/
void QtUiTestWidgets::keyClick(Qt::Key key, Qt::KeyboardModifiers mod,
        QtUiTest::InputOption opt)
{
    QTT_TRACE_FUNCTION();
    int delay;
    if ((delay = qtUiTestInputDelay()) != -1) {
        QtUiTestKeyEvent event = {QtUiTestClick, key, mod, opt};
        d->pendingKeyEvents << event;
        QTimer::singleShot(delay, this, SLOT(_q_postNextKeyEvent()));
        return;
    }

    QTWOptStack st(d, opt);
    d->input->keyClick(key, mod);
}

void QtUiTestWidgetsPrivate::_q_postNextMouseEvent()
{
    QTT_TRACE_FUNCTION();
    QtUiTestMouseEvent const event = pendingMouseEvents.takeAt(0);
    QTWOptStack st(this, event.opt);
    if (event.type == QtUiTestPress)
        input->mousePress(event.pos, event.state);
    else if (event.type == QtUiTestRelease)
        input->mouseRelease(event.pos, event.state);
    else if (event.type == QtUiTestClick)
        input->mouseClick(event.pos, event.state);
    else if (event.type == QtUiTestDClick)
        input->mouseDClick(event.pos, event.state);
    else if (event.type == QtUiTestMove)
        input->mouseMove(event.pos);
}

void QtUiTestWidgetsPrivate::_q_postNextTouchEvent()
{
    QTT_TRACE_FUNCTION();
    QtUiTestTouchEvent const event = pendingTouchEvents.takeAt(0);
    if (event.type == QtUiTestPress)
        input->touchPress(event.points);
    else if (event.type == QtUiTestRelease)
        input->touchRelease(event.points);
    else if (event.type == QtUiTestMove)
        input->touchUpdate(event.points);
}

void QtUiTestWidgetsPrivate::_q_postNextKeyEvent()
{
    QTT_TRACE_FUNCTION();
    QtUiTestKeyEvent const event = pendingKeyEvents.takeAt(0);
    QTWOptStack st(this, event.opt);
    if (event.type == QtUiTestPress)
        input->keyPress(event.key, event.mod);
    else if (event.type == QtUiTestRelease)
        input->keyRelease(event.key, event.mod);
    else if (event.type == QtUiTestClick)
        input->keyClick(event.key, event.mod);
}

/*!
    Set or clear the specified \a option for subsequent simulated input
    events.  The option is set if \a on is true, otherwise it is cleared.
*/
void QtUiTestWidgets::setInputOption(QtUiTest::InputOption option, bool on)
{
    QTT_TRACE_FUNCTION();
    if (on)
        d->inputOptions |= option;
    else
        d->inputOptions &= (~option);
    qttDebug() << "Set input options to" << d->inputOptions;
}

/*!
    Returns true if \a option is currently set.
*/
bool QtUiTestWidgets::testInputOption(QtUiTest::InputOption option) const
{
    QTT_TRACE_FUNCTION();
    return (option == d->inputOptions)
        || (option & d->inputOptions);
}

#include "moc_qtuitestwidgets_p.cpp"
