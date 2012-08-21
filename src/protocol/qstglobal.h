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


#ifndef QST_GLOBAL_H
#define QST_GLOBAL_H

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
