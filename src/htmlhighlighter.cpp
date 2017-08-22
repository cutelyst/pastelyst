/***************************************************************************
 *   Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/
#include "htmlhighlighter.h"

#include <KF5/KSyntaxHighlighting/State>
#include <KF5/KSyntaxHighlighting/Format>
#include <KF5/KSyntaxHighlighting/Theme>
#include <KF5/KSyntaxHighlighting/Definition>
#include <KF5/KSyntaxHighlighting/Repository>

#include <QColor>

#include <QDebug>

using namespace KSyntaxHighlighting;

HtmlHighlighter::HtmlHighlighter() :
    m_repository(new Repository)
{
    for (const Definition &def : m_repository->definitions()) {
        m_languages.push_back(def.name());
        m_definitions.push_front({
                                     { QStringLiteral("id"), def.name() },
                                     { QStringLiteral("name"), def.translatedName() }
                                 });
    }
}

HtmlHighlighter::~HtmlHighlighter()
{
    delete m_repository;
}

void HtmlHighlighter::printDefinitions()
{
    for (const Definition &def : m_repository->definitions()) {
        qDebug() << def.name() << def.section() << def.translatedName();
    }

    for (const Theme &theme : m_repository->themes()) {
        qDebug() << theme.name() << theme.translatedName();
    }
}

QStringList HtmlHighlighter::languages() const
{
    return m_languages;
}

DataList HtmlHighlighter::definitions() const
{
    return m_definitions;
}

QString HtmlHighlighter::highlightString(const QString &definitionName, const QString &themeName, QString *data)
{
    setDefinition(m_repository->definitionForName(definitionName));
    setTheme(m_repository->theme(themeName));

    QString ret;
    ret.reserve(data->size() * 1.5);
    m_out.setString(&ret);

    State state;
    m_out << QStringLiteral("<pre>\n");

    QTextStream in(data, QIODevice::ReadOnly);
    in.setCodec("UTF-8");
    while (!in.atEnd()) {
        m_currentLine = in.readLine();
        state = highlightLine(m_currentLine, state);
        m_out << QLatin1Char('\n');
    }

    m_out << QStringLiteral("</pre>");
    m_out.reset();

    return ret;
}

void HtmlHighlighter::applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format)
{
    const Theme currentTheme = theme();
    if (!format.isDefaultTextStyle(currentTheme)) {
        m_out << QStringLiteral("<span style=\"");
        if (format.hasTextColor(currentTheme))
            m_out << QStringLiteral("color:") << format.textColor(currentTheme).name() << QLatin1Char(';');
        if (format.hasBackgroundColor(currentTheme))
            m_out << QStringLiteral("background-color:") << format.backgroundColor(currentTheme).name() << QLatin1Char(';');
        if (format.isBold(currentTheme))
            m_out << QStringLiteral("font-weight:bold;");
        if (format.isItalic(currentTheme))
            m_out << QStringLiteral("font-style:italic;");
        if (format.isUnderline(currentTheme))
            m_out << QStringLiteral("text-decoration:underline;");
        if (format.isStrikeThrough(currentTheme))
            m_out << QStringLiteral("text-decoration:line-through;");
        m_out << QStringLiteral("\">");
    }

    m_out << m_currentLine.mid(offset, length).toHtmlEscaped();

    if (!format.isDefaultTextStyle(currentTheme)) {
        m_out << QStringLiteral("</span>");
    }
}
