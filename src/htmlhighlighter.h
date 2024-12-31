/***************************************************************************
 *   Copyright (C) 2017-2024 Daniel Nicoletti <dantti12@gmail.com>         *
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
#ifndef HTMLHIGHLIGHTER_H
#define HTMLHIGHLIGHTER_H

#include <KSyntaxHighlighting/AbstractHighlighter>

#include <QTextStream>
#include <QVector>

typedef QVector<QHash<QString, QString> > DataList;

namespace KSyntaxHighlighting {
class Repository;
}

class HtmlHighlighter : public KSyntaxHighlighting::AbstractHighlighter
{
public:
    HtmlHighlighter();
    virtual ~HtmlHighlighter() override;

    void printDefinitions();

    QStringList languages() const;

    DataList definitions() const;

    DataList expirations() const;

    QVector<int> expirationsVector() const;

    QString highlightString(const QString &definitionName, const QString &themeName, QString *data);

protected:
    void applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format) override;

private:
    void pushExpirations(const QString &seconds, const QString &text);

    QStringList m_languages;
    DataList m_definitions;
    DataList m_expirations;
    QVector<int> m_expirationsVector;
    QTextStream m_out;
    QString m_currentLine;
    KSyntaxHighlighting::Repository *m_repository;
};

Q_DECLARE_METATYPE(DataList)

#endif // HTMLHIGHLIGHTER_H
