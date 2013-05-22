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

#include "qstlocalprocess.h"

#include <QtCore/QTextCodec>

QstLocalProcess::QstLocalProcess(QObject *parent) :
    QProcess(parent),
    m_inputCodec(0),
    m_outputCodec(0),
    m_outputDecoder(0),
    m_errorDecoder(0),
    m_outputMode(DeviceOutput)
{
    m_inputCodec = QTextCodec::codecForLocale();
    m_outputCodec = QTextCodec::codecForLocale();

    connect(this, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(forwardFinished(int, QProcess::ExitStatus)));
}

void QstLocalProcess::start(const QString &program, const QStringList &arguments)
{
    QProcess::start(program, arguments, QIODevice::ReadWrite);
}

bool QstLocalProcess::waitForStarted(int msecs)
{
    return QProcess::waitForStarted(msecs);
}

bool QstLocalProcess::waitForFinished(int msecs)
{
    return QProcess::waitForFinished(msecs);
}

bool QstLocalProcess::waitForReadyRead(int msecs)
{
    return QProcess::waitForReadyRead(msecs);
}

void QstLocalProcess::closeWriteChannel()
{
    QProcess::closeWriteChannel();
}

QProcess::ProcessState QstLocalProcess::state() const
{
    return QProcess::state();
}

QProcess::ExitStatus QstLocalProcess::exitStatus() const
{
    return QProcess::exitStatus();
}

int QstLocalProcess::exitCode() const
{
    return QProcess::exitCode();
}

QString QstLocalProcess::inputCodec() const
{
    return QString::fromLatin1(m_inputCodec->name());
}

void QstLocalProcess::setInputCodec(const QString &name)
{
    m_inputCodec = QTextCodec::codecForName(name.toLatin1().constData());
}

QString QstLocalProcess::outputCodec() const
{
    return QString::fromLatin1(m_outputCodec->name());
}

void QstLocalProcess::setOutputCodec(const QString &name)
{
    if (name == outputCodec())
        return;
    m_outputCodec = QTextCodec::codecForName(name.toLatin1().constData());
    delete m_outputDecoder;
    delete m_errorDecoder;
    m_outputDecoder = 0;
    m_errorDecoder = 0;
}

void QstLocalProcess::setOutputMode(OutputMode mode)
{
    if (mode == m_outputMode)
        return;
    m_outputMode = mode;
    if (m_outputMode == DeviceOutput) {
        disconnect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(updateOutputBuffer()));
        disconnect(this, SIGNAL(readyReadStandardError()), this, SLOT(updateErrorBuffer()));
    } else {
        setProcessChannelMode(SeparateChannels);
        connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(updateOutputBuffer()));
        connect(this, SIGNAL(readyReadStandardError()), this, SLOT(updateErrorBuffer()));
        updateOutputBuffer();
        updateErrorBuffer();
    }
}
void QstLocalProcess::updateOutputBuffer()
{
    QByteArray bytes = readAllStandardOutput();
    if (!bytes.isEmpty()) {
        if (!m_outputDecoder)
            m_outputDecoder = m_outputCodec->makeDecoder();
        m_stdoutBuffer.append(m_outputDecoder->toUnicode((bytes)));
        emit standardOutputChanged(m_stdoutBuffer);
    }
}

void QstLocalProcess::updateErrorBuffer()
{
    QByteArray bytes = readAllStandardOutput();
    if (!bytes.isEmpty()) {
        if (!m_errorDecoder)
            m_errorDecoder = m_outputCodec->makeDecoder();
        m_stderrBuffer.append(m_errorDecoder->toUnicode((bytes)));
        emit standardErrorChanged(m_stderrBuffer);
    }
}

void QstLocalProcess::write(const QString &input)
{
    QProcess::write(m_inputCodec->fromUnicode(input));
}
