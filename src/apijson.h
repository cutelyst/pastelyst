/***************************************************************************
 *   Copyright (C) 2017-2019 Daniel Nicoletti <dantti12@gmail.com>         *
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
#ifndef APIJSON_H
#define APIJSON_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

class HtmlHighlighter;
class ApiJson : public Controller
{
    Q_OBJECT
public:
    explicit ApiJson(HtmlHighlighter *htmlHighlighter, QObject *parent = 0);

    C_ATTR(create, :Local :AutoArgs)
    void create(Context *c);

    C_ATTR(show, :Local :AutoArgs)
    void show(Context *c, const QStringList &args);

    C_ATTR(list, :Local :AutoArgs)
    void list(Context *c, const QString &page);

    C_ATTR(parameterExpire, :Path('parameter/expire') :AutoArgs)
    void parameterExpire(Context *c);

    C_ATTR(parameterLanguage, :Path('parameter/language') :AutoArgs)
    void parameterLanguage(Context *c);

    C_ATTR(parameterVersion, :Path('parameter/version') :AutoArgs)
    void parameterVersion(Context *c);

    C_ATTR(parameterTheme, :Path('parameter/theme') :AutoArgs)
    void parameterTheme(Context *c);

private:
    HtmlHighlighter *m_htmlHighlighter;
};

#endif // APIJSON_H
