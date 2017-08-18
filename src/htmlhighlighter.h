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
#ifndef HTMLHIGHLIGHTER_H
#define HTMLHIGHLIGHTER_H

#include <KF5/KSyntaxHighlighting/AbstractHighlighter>

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
    ~HtmlHighlighter();

    void printDefinitions();

    DataList definitions() const;

    QString highlightString(const QString &definitionName, const QString &themeName, QString *data);

protected:
    void applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format) override;

private:
    QVector<QHash<QString, QString> > m_definitions;
    QTextStream m_out;
    QString m_currentLine;
    KSyntaxHighlighting::Repository *m_repository;
};

Q_DECLARE_METATYPE(DataList)

#endif // HTMLHIGHLIGHTER_H
