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

#include "qstlogwriter.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QList>


#include <QtCore/QDebug>

static QIODevice *createDevice(const QString &path)
{
    QFile *file = new QFile;
    if (path.isEmpty()) {
        if (!file->open(stdout, QFile::WriteOnly)) {
            qWarning("Error opening stdout: %s", qPrintable(file->errorString()));
            delete file;
            file = 0;
        }
    } else {
        file->setFileName(path);
        if (!file->open(QFile::WriteOnly)) {
            qWarning("Error opening log file %s: %s", qPrintable(path), qPrintable(file->errorString()));
            delete file;
            file = 0;
        }
    }
    return file;
}


void QstPlainTextWriter::setFilePath(const QString &path)
{
    QIODevice *dev = m_stream->device();
    m_stream->setDevice(0);
    dev->deleteLater();

    dev = createDevice(path);
    if (dev)
        m_stream->setDevice(dev);
}

QString QstPlainTextWriter::filePath() const
{
    QFile *file = qobject_cast<QFile*>(m_stream->device());
    if (file) {
        QFileInfo info(*file);
        return info.absoluteFilePath();
    }
    return QString();
}

void QstPlainTextWriter::writeLine(const QString &text)
{
    *m_stream << text << endl;
    m_stream->flush();
}

static QList<int> color_codes(QstPlainTextWriter::Color color, bool foreground = true)
{
    QList<int> commands;
    if (color == QstPlainTextWriter::DefaultColor)
        return commands;
    int distance = (int)color - (int)QstPlainTextWriter::Black;
    commands.append(distance % 2);
    commands.append((distance / 2) + (foreground ? 30 : 40));
    return commands;
}

QString QstPlainTextWriter::colorize(const QString &text, Color foreground, Color background)
{
    if (m_colorSetting == NeverColor || (m_colorSetting == AutoColor && !isConsole()))
        return text;

    QList<int> commands;
    commands << color_codes(foreground, true) << color_codes(background, false);
    if (commands .isEmpty())
        return text;
    QString result = QString::fromLatin1("\033[");
    QListIterator<int> it(commands);
    while(it.hasNext()) {
        result += QString::number(it.next());
        if (it.hasNext())
            result += QString::fromLatin1(";");
    }
    result += "m";
    return result + text + "\033[0m";
}

bool QstPlainTextWriter::isConsole() const
{
    QFile *file = qobject_cast<QFile*>(m_stream->device());
    if (file) {
        // not sure, but may need to use _isatty on win
        return isatty(file->handle());
    }
    return false;
}

void QstXmlStreamWriter::setFilePath(const QString &path)
{
    QIODevice *dev = m_writer->device();
    m_writer->setDevice(0);
    dev->deleteLater();

    dev = createDevice(path);
    if (dev)
        m_writer->setDevice(dev);
}

QString QstXmlStreamWriter::filePath() const
{
    QFile *file = qobject_cast<QFile*>(m_writer->device());
    if (file) {
        QFileInfo info(*file);
        return info.absoluteFilePath();
    }
    return QString();
}

void QstXmlStreamWriter::flush()
{
    QFile *file = qobject_cast<QFile *>(m_writer->device());
    if (file)
        file->flush();
}
