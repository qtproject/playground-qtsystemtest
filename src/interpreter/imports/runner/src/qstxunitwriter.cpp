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

#include "qstxunitwriter.h"
#include <QtCore/QDateTime>


QstXUnitWriter::QstXUnitWriter()
    : QstXmlStreamWriter(),
      m_state(Document),
      m_suiteId(1)
{
}

QstXUnitWriter::~QstXUnitWriter()
{
}

bool QstXUnitWriter::checkState(State state, const QString &element, bool start)
{
    if (m_state != state) {
        if (m_state != Error) {
            if (start)
                qWarning("unexpected state when starting '%s'", element.toLocal8Bit().constData());
            else
                qWarning("unexpected state when ending '%s'", element.toLocal8Bit().constData());
            m_state = Error;
        }
        return false;
    }
    if (hasError()) {
        if (m_state != Error) {
            qWarning("unexpected error with device");
            m_state = Error;
        }
        return false;
    }
    return true;
}

static const QString s_suiteSet(QString::fromLatin1("testsuites"));
static const QString s_suite(QString::fromLatin1("testsuite"));
static const QString s_case(QString::fromLatin1("testcase"));
static const QString s_error(QString::fromLatin1("error"));
static const QString s_failure(QString::fromLatin1("failure"));
static const QString s_skipped(QString::fromLatin1("skipped"));

void QstXUnitWriter::writeStartTestSuiteSet()
{
    if (!checkState(Document, s_suiteSet, true))
        return;
    m_state = SuiteSet;

    writeStartElement(s_suiteSet);
}

void QstXUnitWriter::writeEndTestSuiteSet()
{
    if (!checkState(SuiteSet, s_suiteSet, false))
        return;
    m_state = Document;

    writeEndElement();
    flush();
}

void QstXUnitWriter::writeStartTestSuite(
        const QString &name,
        const QDateTime &timestamp,
        const QString &hostname,
        int testTotal, int testFails, int testErrors, int testSkips,
        int duration)
{
    if (!checkState(SuiteSet, s_suite, true))
        return;
    m_state = Suite;

    writeStartElement(s_suite);
    writeAttribute("name", name);
    writeAttribute("package", QString::fromLatin1("none"));
    writeAttribute("timestamp", timestamp.toLocalTime().toString("yyyy-MM-ddThh:mm:ss"));
    writeAttribute("hostname", hostname.isEmpty() ? "localhost" : hostname);
    writeAttribute("tests", QString::number(testTotal));
    writeAttribute("failures", QString::number(testFails));
    writeAttribute("errors", QString::number(testErrors));
    writeAttribute("skipped", QString::number(testSkips));
    writeAttribute("time", QString::number(duration));
    writeAttribute("id", QString::number(m_suiteId++));
}

void QstXUnitWriter::writeEndTestSuite()
{
    if (!checkState(Suite, s_suite, false))
        return;
    m_state = SuiteSet;

    writeEndElement();
}


void QstXUnitWriter::writeStartTestCase(const QString &classname, const QString &name, int duration)
{
    if (!checkState(Suite, s_case, true))
        return;
    m_state = Case;

    writeStartElement(s_case);
    writeAttribute("classname", classname);
    writeAttribute("name", name);
    writeAttribute("time", QString::number(duration));
}

void QstXUnitWriter::writeEndTestCase()
{
    if (!checkState(Case, s_case, false))
        return;
    m_state = Suite;

    writeEndElement();
}

void QstXUnitWriter::writeError(const QString &type, const QString &message)
{
    if (!checkState(Case, s_error, true))
        return;

    writeStartElement(s_error);
    writeAttribute("type", type);
    writeCharacters(message);
    writeEndElement();
}
void QstXUnitWriter::writeFailure(const QString &type, const QString &message)
{
    if (!checkState(Case, s_failure, true))
        return;

    writeStartElement(s_failure);
    writeAttribute("type", type);
    writeCharacters(message);
    writeEndElement();
}
void QstXUnitWriter::writeSkipped(const QString &type, const QString &message)
{
    if (!checkState(Case, s_skipped, true))
        return;

    writeStartElement(s_skipped);
    writeAttribute("type", type);
    writeCharacters(message);
    writeEndElement();
}
