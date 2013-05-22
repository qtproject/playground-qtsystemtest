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

#ifndef QSTLOGWRITER_H
#define QSTLOGWRITER_H

#include <QtCore/QTextStream>
#include <QtCore/QXmlStreamWriter>


class QstPlainTextWriter : public QObject
{
    Q_OBJECT
    Q_ENUMS(ColorSetting Color)
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath)
    Q_PROPERTY(ColorSetting colorSetting READ colorSetting WRITE setColorSetting)
public:
    enum ColorSetting {
        AutoColor,
        AlwaysColor,
        NeverColor
    };

    enum Color {
        DefaultColor,
        Black, // the order of these colors matters, see colorize
        DarkGray,
        Red,
        LightRed,
        Green,
        LightGreen,
        Brown,
        Yellow,
        Blue,
        LightBlue,
        Purple,
        LightPurple,
        Cyan,
        LightCyan,
        LightGray,
        White
    };

    QstPlainTextWriter(QObject *parent = 0)
        : QObject(parent),
          m_colorSetting(AutoColor)
    {
        m_stream = new QTextStream;
        m_stream->setCodec("UTF8");
    }

    ~QstPlainTextWriter() {
        if (m_stream->device()) {
            m_stream->device()->close();
            m_stream->device()->deleteLater();
        }
        delete m_stream;
    }

    void setColorSetting(ColorSetting setting) { m_colorSetting = setting; }
    ColorSetting colorSetting() const { return m_colorSetting; }

    void setFilePath(const QString &);
    QString filePath() const;

    Q_INVOKABLE void writeLine(const QString &);

    Q_INVOKABLE QString colorize(const QString &text, Color foreground = DefaultColor, Color background = DefaultColor);

private:
    Q_INVOKABLE bool isConsole() const;
    QTextStream *m_stream;
    ColorSetting m_colorSetting;
};

class QstXmlStreamWriter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath)
    Q_PROPERTY(bool autoFormatting READ autoFormatting WRITE setAutoFormatting)
    Q_PROPERTY(int autoFormattingIndent READ autoFormattingIndent WRITE setAutoFormattingIndent)

public:
    QstXmlStreamWriter(QObject *parent = 0)
        : QObject(parent)
    {
        m_writer = new QXmlStreamWriter;
        m_writer->setAutoFormatting(true);
    }

    ~QstXmlStreamWriter()
    {
        flush();
        if (m_writer->device()) {
            m_writer->device()->close();
            m_writer->device()->deleteLater();
        }
        delete m_writer;
    }

    void setFilePath(const QString &);
    QString filePath() const;

    void setAutoFormatting(bool value)
    { m_writer->setAutoFormatting(value); }
    bool autoFormatting() const
    { return m_writer->autoFormatting(); }

    void setAutoFormattingIndent(int spacesOrTabs)
    { m_writer->setAutoFormattingIndent(spacesOrTabs); }
    int autoFormattingIndent() const
    { return m_writer->autoFormattingIndent(); }

    Q_INVOKABLE void writeAttribute(const QString &qualifiedName, const QString &value)
    { m_writer->writeAttribute(qualifiedName, value); }
    Q_INVOKABLE void writeAttribute(const QString &namespaceUri, const QString &name, const QString &value)
    { m_writer->writeAttribute(namespaceUri, name, value); }

    Q_INVOKABLE void writeCDATA(const QString &text)
    { m_writer->writeCDATA(text); }
    Q_INVOKABLE void writeCharacters(const QString &text)
    { m_writer->writeCharacters(text); }
    Q_INVOKABLE void writeComment(const QString &text)
    { m_writer->writeComment(text); }

    Q_INVOKABLE void writeDTD(const QString &dtd)
    { m_writer->writeDTD(dtd); }

    Q_INVOKABLE void writeEmptyElement(const QString &qualifiedName)
    { m_writer->writeEmptyElement(qualifiedName); }
    Q_INVOKABLE void writeEmptyElement(const QString &namespaceUri, const QString &name)
    { m_writer->writeEmptyElement(namespaceUri, name); }

    Q_INVOKABLE void writeTextElement(const QString &qualifiedName, const QString &text)
    { m_writer->writeTextElement(qualifiedName, text); }
    Q_INVOKABLE void writeTextElement(const QString &namespaceUri, const QString &name, const QString &text)
    { m_writer->writeTextElement(namespaceUri, name, text); }

    Q_INVOKABLE void writeEndDocument()
    { m_writer->writeEndDocument(); flush(); }
    Q_INVOKABLE void writeEndElement()
    { m_writer->writeEndElement(); }

    Q_INVOKABLE void writeEntityReference(const QString &name)
    { m_writer->writeEntityReference(name); }
    Q_INVOKABLE void writeNamespace(const QString &namespaceUri, const QString &prefix = QString())
    { m_writer->writeNamespace(namespaceUri, prefix); }
    Q_INVOKABLE void writeDefaultNamespace(const QString &namespaceUri)
    { m_writer->writeDefaultNamespace(namespaceUri); }
    Q_INVOKABLE void writeProcessingInstruction(const QString &target, const QString &data = QString())
    { m_writer->writeProcessingInstruction(target, data); }

    Q_INVOKABLE void writeStartDocument()
    { m_writer->writeStartDocument(); }
    Q_INVOKABLE void writeStartDocument(const QString &version)
    { m_writer->writeStartDocument(version); }
    Q_INVOKABLE void writeStartDocument(const QString &version, bool standalone)
    { m_writer->writeStartDocument(version, standalone); }
    Q_INVOKABLE void writeStartElement(const QString &qualifiedName)
    { m_writer->writeStartElement(qualifiedName); }
    Q_INVOKABLE void writeStartElement(const QString &namespaceUri, const QString &name)
    { m_writer->writeStartElement(namespaceUri, name); }

    Q_INVOKABLE bool hasError() const
    { return m_writer->hasError(); }

    void flush();
private:

    QXmlStreamWriter *m_writer;
};


#endif // QSTLOGWRITER_H
