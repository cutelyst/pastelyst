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
        m_definitions.push_back({
                                    { QStringLiteral("id"), def.name() },
                                    { QStringLiteral("name"), def.translatedName() }
                                });
    }

    std::sort(m_definitions.begin(), m_definitions.end(), [] (const QHash<QString, QString> &a, const QHash<QString, QString> &b) -> bool {
        return a.value(QStringLiteral("name")).compare(b.value(QStringLiteral("name")), Qt::CaseInsensitive) < 0;
    });

    m_languages.push_front(QStringLiteral("text"));
    m_definitions.push_front({
                                { QStringLiteral("id"), QStringLiteral("text") },
                                { QStringLiteral("name"), QStringLiteral("Text") }
                            });

    pushExpirations(QStringLiteral("1800"), QStringLiteral("for 30 minutes"));
    pushExpirations(QStringLiteral("21600"), QStringLiteral("for 6 hours"));
    pushExpirations(QStringLiteral("86400"), QStringLiteral("for 1 day"));
    pushExpirations(QStringLiteral("604800"), QStringLiteral("for 1 week"));
    pushExpirations(QStringLiteral("2592000"), QStringLiteral("for 1 month"));
    pushExpirations(QStringLiteral("31536000"), QStringLiteral("for 1 year"));
    pushExpirations(QStringLiteral("0"), QStringLiteral("forever"));
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

DataList HtmlHighlighter::expirations() const
{
    return m_expirations;
}

QVector<int> HtmlHighlighter::expirationsVector() const
{
    return m_expirationsVector;
}

QString HtmlHighlighter::highlightString(const QString &definitionName, const QString &themeName, QString *data)
{
    setDefinition(m_repository->definitionForName(definitionName));
    setTheme(m_repository->theme(themeName));

    QString ret;
    ret.reserve(data->size() * 1.5);
    m_out.setString(&ret);

    State state;

    QTextStream in(data, QIODevice::ReadOnly);
    in.setCodec("UTF-8");
    while (!in.atEnd()) {
        m_currentLine = in.readLine();
        state = highlightLine(m_currentLine, state);
        m_out << QLatin1Char('\n');
    }

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

void HtmlHighlighter::pushExpirations(const QString &seconds, const QString &text)
{
    m_expirationsVector.push_back(seconds.toInt());
    m_expirations.push_back({
                                 { QStringLiteral("id"), seconds },
                                 { QStringLiteral("name"), text }
                             });
}
