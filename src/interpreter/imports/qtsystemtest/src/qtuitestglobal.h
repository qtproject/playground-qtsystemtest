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


#ifndef QTUITEST_GLOBAL_H
#define QTUITEST_GLOBAL_H

#include <QtGlobal>

#if defined(Q_OS_WIN32)
# if defined(QTUITEST_TARGET)
#  define QTUITEST_EXPORT Q_DECL_EXPORT
# else
#  define QTUITEST_EXPORT Q_DECL_IMPORT
# endif
# if defined(QTSLAVE_TARGET)
#  define QTSLAVE_EXPORT Q_DECL_EXPORT
# else
#  define QTSLAVE_EXPORT Q_DECL_IMPORT
# endif
#else
# define QTUITEST_EXPORT Q_DECL_EXPORT
# define QTSLAVE_EXPORT Q_DECL_EXPORT
#endif

#if defined(Q_OS_WIN32) || defined(Q_OS_SYMBIAN)
# if defined(QSYSTEMTEST_TARGET)
#  define QSYSTEMTEST_EXPORT Q_DECL_EXPORT
# else
#  define QSYSTEMTEST_EXPORT Q_DECL_IMPORT
# endif
# if defined(QTUITESTRUNNER_TARGET)
#  define QTUITESTRUNNER_EXPORT Q_DECL_EXPORT
# else
#  define QTUITESTRUNNER_EXPORT Q_DECL_IMPORT
# endif
#else
# define QSYSTEMTEST_EXPORT Q_DECL_EXPORT
# define QTUITESTRUNNER_EXPORT Q_DECL_EXPORT
#endif

#define xstr(s) str(s)
#define str(s) #s
#define QTUITEST_VERSION_STR xstr(QTUITEST_VERSION)

#endif
