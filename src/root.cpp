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
#include "root.h"

#include "htmlhighlighter.h"

#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Utils/Pagination>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QUuid>

#include <QDebug>

using namespace Cutelyst;

Root::Root(HtmlHighlighter *htmlHighlighter, QObject *parent) : Controller(parent),
    m_htmlHighlighter(htmlHighlighter)
{

}

Root::~Root()
{
}

void Root::index(Context *c)
{
    c->setStash(QStringLiteral("languages"), QVariant::fromValue(m_htmlHighlighter->definitions()));
}

void Root::item(Context *c, const QString &uuid)
{
    QSqlQuery query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("SELECT uuid, title, html, language, ip_address, user_agent, private, expires_at, created_at "
                               "FROM notes "
                               "WHERE uuid = :uuid"),
                QStringLiteral("sticklyst"));
    query.bindValue(QStringLiteral(":uuid"), uuid);
    if (query.exec() && query.size()) {
        QVariantHash obj = Sql::queryToHashObject(query);
        QDateTime dt = QDateTime::fromString(obj.value(QStringLiteral("created_at")).toString(), Qt::ISODate);
        dt.setTimeSpec(Qt::LocalTime);
        obj.insert(QStringLiteral("created_at"), dt);
        c->setStash(QStringLiteral("note"), obj);
        qDebug() << "EXEC" << dt;
    } else {
        c->forward(CActionFor(QStringLiteral("notFound")));
    }
}

void Root::raw(Context *c, const QString &uuid)
{
    QSqlQuery query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("SELECT uuid, title, raw, created_at "
                               "FROM notes "
                               "WHERE uuid = :uuid"),
                QStringLiteral("sticklyst"));
    query.bindValue(QStringLiteral(":uuid"), uuid);
    if (query.exec() && query.next()) {
        const QString raw = query.value(2).toString();
        QDateTime dt = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
        dt.setTimeSpec(Qt::LocalTime);

        c->response()->headers().setLastModified(dt);
        c->response()->setBody(raw);
        c->response()->setContentType(QStringLiteral("text/plain; charset=UTF-8"));
        qDebug() << "EXEC" << dt;
    } else {
        c->forward(CActionFor(QStringLiteral("notFound")));
    }
}

void Root::create(Context *c)
{
    if (!c->request()->isPost()) {
        c->res()->redirect(c->uriFor(actionFor(QStringLiteral("index"))));
        return;
    }

    const ParamsMultiMap params = c->request()->bodyParams();
    QString uuid;
    createNote(c, m_htmlHighlighter, params, uuid);

    c->res()->redirect(c->uriFor(CActionFor(QStringLiteral("item")), QStringList{ uuid }));
}

bool Root::createNote(Context *c, HtmlHighlighter *htmlHighlighter, const ParamsMultiMap &params, QString &result)
{
    const QString title = params.value(QStringLiteral("title"));
    const QString language = params.value(QStringLiteral("language"));
    const QString expire = params.value(QStringLiteral("expire"));
    const QString priv = params.value(QStringLiteral("private"));

    QString data = params.value(QStringLiteral("data"));
    const QString dataHighlighted = htmlHighlighter->highlightString(language, QStringLiteral("Default"), &data);

    int pos = 0;
    int count = 0;
    while (pos != -1 && pos < dataHighlighted.size() && count < 6) {
        pos = dataHighlighted.indexOf(QLatin1Char('\n'), pos + 1);
        ++count;
    }

    QString shortHtml;
    if (count == 6) {
        shortHtml = dataHighlighted.mid(0, pos) + QLatin1String("\n</pre>");
    } else {
        shortHtml = dataHighlighted;
    }

    // LEFT must be greater than all sticklyst actions ie localhost/some_length_action
    const QString uuid = QString::fromLatin1(
                QUuid::createUuid().toRfc4122()
                .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals).left(9));

    QSqlQuery query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("INSERT INTO notes "
                               "(uuid, title, raw, html, short, language, ip_address, user_agent, private, expires_at, created_at) "
                               "VALUES "
                               "(:uuid, :title, :raw, :html, :short, :language, :ip_address, :user_agent, :private, :expires_at, :created_at)"),
                QStringLiteral("sticklyst"));
    query.bindValue(QStringLiteral(":uuid"), uuid);
    query.bindValue(QStringLiteral(":title"), title);
    query.bindValue(QStringLiteral(":raw"), data);
    query.bindValue(QStringLiteral(":html"), dataHighlighted);
    query.bindValue(QStringLiteral(":short"), shortHtml);
    query.bindValue(QStringLiteral(":language"), language);
    query.bindValue(QStringLiteral(":ip_address"), c->request()->addressString());
    query.bindValue(QStringLiteral(":user_agent"), c->request()->userAgent());
    query.bindValue(QStringLiteral(":private"), !priv.isEmpty());
    query.bindValue(QStringLiteral(":expires_at"), 3600);
    query.bindValue(QStringLiteral(":created_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    if (query.exec() && query.numRowsAffected() == 1) {
        qDebug() << "Created Paste" << uuid;
        result = uuid;
        return true;
    }
    result = query.lastError().databaseText();
    qDebug() << "Failed Creating Paste" << uuid << result;

    return false;
}

void Root::all(Context *c)
{
    const int notesPerPage = 10;
    int offset;

    QSqlQuery query;
    query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("SELECT count(*) FROM notes"),
                QStringLiteral("sticklyst"));
    if (Q_LIKELY(query.exec() && query.next())) {
        int rows = query.value(0).toInt();
        Pagination pagination(rows,
                              notesPerPage,
                              c->req()->queryParam(QStringLiteral("page"), QStringLiteral("1")).toInt());
        offset = pagination.offset();
        c->setStash(QStringLiteral("pagination"), pagination);
        c->setStash(QStringLiteral("posts_count"), rows);
    } else {
        c->response()->redirect(c->uriFor(CActionFor(QStringLiteral("notFound"))));
        return;
    }

    query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("SELECT uuid, title, short, language, ip_address, user_agent, private, expires_at, created_at "
                               "FROM notes "
                               "ORDER BY id DESC "
                               "LIMIT :limit OFFSET :offset"),
                QStringLiteral("sticklyst"));
    query.bindValue(QStringLiteral(":limit"), notesPerPage);
    query.bindValue(QStringLiteral(":offset"), offset);
    if (query.exec()) {
        QVariantList list = Sql::queryToHashList(query);
        auto it = list.begin();
        while (it != list.end()) {
            QVariantHash obj = (*it).toHash();
            QDateTime dt = QDateTime::fromString(obj.value(QStringLiteral("created_at")).toString(), Qt::ISODate);
            dt.setTimeSpec(Qt::LocalTime);
            obj.insert(QStringLiteral("created_at"), dt);
            *it = obj;
            ++it;
        }
        c->setStash(QStringLiteral("notes"), list);
    }
}

void Root::notFound(Context *c)
{
    c->response()->setStatus(404);
}

