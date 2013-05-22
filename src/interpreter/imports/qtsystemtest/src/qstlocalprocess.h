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

#ifndef QSTLOCALPROCESS_H
#define QSTLOCALPROCESS_H

#include <QtCore/QProcess>

class QTextCodec;
class QTextDecoder;

class QstLocalProcess : public QProcess
{
    Q_OBJECT
    Q_ENUMS(ProcessState ExitStatus OutputMode)

    Q_PROPERTY(ProcessState state READ state NOTIFY stateChanged)
    Q_PROPERTY(ExitStatus exitStatus READ exitStatus NOTIFY exitStatusChanged)
    Q_PROPERTY(int exitCode READ exitCode NOTIFY exitCodeChanged)

    Q_PROPERTY(QString standardOutput READ standardOutput NOTIFY standardOutputChanged)
    Q_PROPERTY(QString standardError READ standardError NOTIFY standardErrorChanged)

    Q_PROPERTY(QString inputCodec READ inputCodec WRITE setInputCodec)
    Q_PROPERTY(QString outputCodec READ outputCodec WRITE setOutputCodec)

    Q_PROPERTY(OutputMode outputMode READ outputMode WRITE setOutputMode)

public:
    explicit QstLocalProcess(QObject *parent = 0);

    enum OutputMode {
        DeviceOutput,
        PropertyOutput
    };

    Q_INVOKABLE void start(const QString &program, const QStringList &arguments = QStringList());
    Q_INVOKABLE bool waitForStarted(int msecs);
    Q_INVOKABLE bool waitForFinished(int msecs);
    Q_INVOKABLE bool waitForReadyRead(int msecs);
    Q_INVOKABLE void closeWriteChannel();

    Q_INVOKABLE ProcessState state() const;
    Q_INVOKABLE ExitStatus exitStatus() const;
    Q_INVOKABLE int exitCode() const;

    Q_INVOKABLE void write(const QString &input);

    QString standardOutput() const { return m_stdoutBuffer; }
    Q_INVOKABLE void clearStandardOutput()
    { m_stdoutBuffer.clear(); emit standardOutputChanged(m_stdoutBuffer); }

    QString standardError() const { return m_stderrBuffer; }
    Q_INVOKABLE  void clearStandardError()
    { m_stderrBuffer.clear(); emit standardErrorChanged(m_stderrBuffer); }

    QString inputCodec() const;
    QString outputCodec() const;
    void setInputCodec(const QString &);
    void setOutputCodec(const QString &);

    OutputMode outputMode() const { return m_outputMode; }
    void setOutputMode(OutputMode mode);

signals:
    void stateChanged(QProcess::ProcessState state);
    void exitStatusChanged(QProcess::ExitStatus status);
    void exitCodeChanged(int);
    void standardOutputChanged(const QString &);
    void standardErrorChanged(const QString &);

private slots:
    void forwardFinished(int code, QProcess::ExitStatus status)
    {
        emit exitStatusChanged(status);
        emit exitCodeChanged(code);
    }

    void updateOutputBuffer();
    void updateErrorBuffer();

private:
    QTextCodec *m_inputCodec;
    QTextCodec *m_outputCodec;
    QTextDecoder *m_outputDecoder;
    QTextDecoder *m_errorDecoder;
    OutputMode m_outputMode;

    QString m_stdoutBuffer;
    QString m_stderrBuffer;
};

#endif // QSTLOCALPROCESS_H
