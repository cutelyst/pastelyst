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
#ifndef ROOT_H
#define ROOT_H

#include <Cutelyst/Controller>

using namespace Cutelyst;

class HtmlHighlighter;
class Root : public Controller
{
    Q_OBJECT
    C_NAMESPACE("")
public:
    explicit Root(QObject *parent = 0);
    ~Root();

    C_ATTR(index, :Path :AutoArgs)
    void index(Context *c);

    C_ATTR(item, :Path :AutoArgs)
    void item(Context *c, const QString &uuid);

    C_ATTR(raw, :Local :AutoArgs)
    void raw(Context *c, const QString &uuid);

    C_ATTR(create, :Local :AutoArgs)
    void create(Context *c);

    C_ATTR(all, :Local :AutoArgs)
    void all(Context *c);

    C_ATTR(notFound, :Path)
    void notFound(Context *c);

private:
    C_ATTR(End, :ActionClass("RenderView"))
    void End(Context *c) { Q_UNUSED(c); }

private:
    HtmlHighlighter *m_htmlHighlighter;
};

#endif //ROOT_H

