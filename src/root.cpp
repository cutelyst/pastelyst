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
#include "root.h"

#include "htmlhighlighter.h"

#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Utils/Pagination>
#include <Cutelyst/Plugins/Authentication/credentialpassword.h>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Application>

#include <grantlee/safestring.h>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QUuid>

#include <QDebug>

using namespace Cutelyst;

Root::Root(HtmlHighlighter *htmlHighlighter, QObject *parent) : Controller(parent),
    m_htmlHighlighter(htmlHighlighter)
{
    m_cleaupTimer.start();
}

Root::~Root()
{
}

void Root::index(Context *c)
{
    c->setStash(QStringLiteral("languages"), QVariant::fromValue(m_htmlHighlighter->definitions()));
    c->setStash(QStringLiteral("expires"), QVariant::fromValue(m_htmlHighlighter->expirations()));
}

void Root::item(Context *c, const QString &uuid)
{
    c->setStash(QStringLiteral("social"), m_socialMediaButtons);
    c->setStash(QStringLiteral("download"), m_downloadButton);
    c->setStash(QStringLiteral("clipboard"), m_clipboardButton);

    QSqlQuery query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("SELECT uuid, title, html, language, ip_address, user_agent, private, expires, created_at, password "
                               "FROM notes "
                               "WHERE uuid = :uuid"),
                QStringLiteral("pastelyst"));

    query.bindValue(QStringLiteral(":uuid"), uuid);
    if (!query.exec()) {
        c->forward(CActionFor(QStringLiteral("notFound")));
        return;
    }

    QVariantHash obj = Sql::queryToHashObject(query);
    if (obj.isEmpty()) {
        c->forward(CActionFor(QStringLiteral("notFound")));
        return;
    }

    const QString password = obj.value(QStringLiteral("password")).toString();

    if (!password.isEmpty() && Session::value(c, uuid).toBool() == false) {
        if (!c->request()->isPost()) {
            c->setStash(QStringLiteral("template"), QStringLiteral("item_password.html"));
            return;
        }

        const QString userPassword = c->request()->bodyParameter(QStringLiteral("password"));
        if (!CredentialPassword::validatePassword(userPassword.toUtf8(), password.toUtf8())) {
            c->setStash(QStringLiteral("template"), QStringLiteral("item_password.html"));
            return;
        }

        Session::setValue(c, uuid, true);
    }

    QDateTime dt = QDateTime::fromString(obj.value(QStringLiteral("created_at")).toString(), Qt::ISODate);
    dt.setTimeSpec(Qt::LocalTime);
    obj.insert(QStringLiteral("created_at"), dt);

    const Grantlee::SafeString html(obj.value(QStringLiteral("html")).toString(), true);
    obj.insert(QStringLiteral("html"), html);

    c->setStash(QStringLiteral("note"), obj);
    c->setStash(QStringLiteral("title"), obj.value(QStringLiteral("title")));
}

void Root::raw(Context *c, const QString &uuid)
{
    QSqlQuery query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("SELECT uuid, title, raw, created_at, password "
                               "FROM notes "
                               "WHERE uuid = :uuid"),
                QStringLiteral("pastelyst"));
    query.bindValue(QStringLiteral(":uuid"), uuid);
    if (query.exec() && query.next()) {
        const QString password = query.value(QStringLiteral("password")).toString();
        if (!password.isEmpty() && Session::value(c, uuid).toBool() == false) {
            c->response()->redirect(c->uriFor(CActionFor(QStringLiteral("item")), QStringList{ uuid }));
            return;
        }

        const QString raw = query.value(2).toString();
        QDateTime dt = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
        dt.setTimeSpec(Qt::LocalTime);

        c->response()->headers().setLastModified(dt);
        c->response()->setBody(raw);
        c->response()->setContentType(QStringLiteral("text/plain; charset=UTF-8"));
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
    QString result;
    if (createNote(c, m_htmlHighlighter, params, result)) {
        Session::setValue(c, result, true);
    } else {
        c->res()->redirect(c->uriFor(actionFor(QStringLiteral("index"))));
        return;
    }

    c->res()->redirect(c->uriFor(CActionFor(QStringLiteral("item")), QStringList{ result }));
}

bool Root::createNote(Context *c, HtmlHighlighter *htmlHighlighter, const ParamsMultiMap &params, QString &result)
{
    const QString title = params.value(QStringLiteral("title"));
    const QString language = params.value(QStringLiteral("language"));
    QString password = params.value(QStringLiteral("password"));
    const bool priv = params.value(QStringLiteral("private")) == QLatin1String("on") || !password.isEmpty();

    const int expire = params.value(QStringLiteral("expire")).toInt();
    if (!htmlHighlighter->expirationsVector().contains(expire)) {
        result = QStringLiteral("Invalid expiration");
        return false;
    }

    if (!password.isEmpty()) {
        password = QString::fromLatin1(CredentialPassword::createPassword(password.toUtf8(),
                                                                          QCryptographicHash::Sha256,
                                                                          100, 24, 24));
    }

    QString data = params.value(QStringLiteral("data"));
    if (data.isEmpty()) {
        result = QStringLiteral("Please post some content");
        return false;
    }

    QString dataHighlighted;
    if (language == QLatin1String("text")) {
        dataHighlighted = data.toHtmlEscaped();
    } else {
        dataHighlighted = htmlHighlighter->highlightString(language, QStringLiteral("Default"), &data);
    }

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

    // LEFT must be greater than all pastelyst actions ie localhost/some_length_action
    const int left = priv ? 13 : 9;
    const QString uuid = QString::fromLatin1(
                QUuid::createUuid().toRfc4122()
                .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals).left(left));

    QSqlQuery query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("INSERT INTO notes "
                               "(uuid, title, raw, html, short, language, ip_address, "
                               " user_agent, private, password, expires, created_at) "
                               "VALUES "
                               "(:uuid, :title, :raw, :html, :short, :language, :ip_address,"
                               " :user_agent, :private, :password, :expires, :created_at)"),
                QStringLiteral("pastelyst"));
    query.bindValue(QStringLiteral(":uuid"), uuid);
    query.bindValue(QStringLiteral(":title"), title);
    query.bindValue(QStringLiteral(":raw"), data);
    query.bindValue(QStringLiteral(":html"), dataHighlighted);
    query.bindValue(QStringLiteral(":short"), shortHtml);
    query.bindValue(QStringLiteral(":language"), language);
    query.bindValue(QStringLiteral(":ip_address"), c->request()->addressString());
    query.bindValue(QStringLiteral(":user_agent"), c->request()->userAgent());
    query.bindValue(QStringLiteral(":private"), priv);
    query.bindValue(QStringLiteral(":password"), password);
    query.bindValue(QStringLiteral(":expires"), expire);
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
                QStringLiteral("SELECT count(*) FROM notes WHERE private == 0"),
                QStringLiteral("pastelyst"));
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
                QStringLiteral("SELECT uuid, title, short, language, ip_address, user_agent, private, expires, created_at "
                               "FROM notes "
                               "WHERE private == 0 "
                               "ORDER BY id DESC "
                               "LIMIT :limit OFFSET :offset"),
                QStringLiteral("pastelyst"));
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

            const Grantlee::SafeString shortHtml(obj.value(QStringLiteral("short")).toString(), true);
            obj.insert(QStringLiteral("short"), shortHtml);

            *it = obj;
            ++it;
        }
        c->setStash(QStringLiteral("notes"), list);
    }
}

void Root::search(Context *c)
{
    const int notesPerPage = 10;
    int offset;

    QSqlQuery query;

    QString searchText = c->req()->queryParam(QStringLiteral("q"));

    query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("SELECT count(*) FROM notes WHERE private == 0 "
                               "AND (title LIKE '%'||:search||'%' "
                               "OR raw LIKE '%'||:search||'%') "),
                QStringLiteral("pastelyst"));
    // https://forum.qt.io/topic/78303/why-doesn-t-bind-work-with-this-query/8
    query.bindValue(QStringLiteral(":search"), searchText);

    if (Q_LIKELY(query.exec() && query.next())) {
        int rows = query.value(0).toInt();
        Pagination pagination(rows,
                              notesPerPage,
                              c->req()->queryParam(QStringLiteral("page"), QStringLiteral("1")).toInt());
        offset = pagination.offset();
        c->setStash(QStringLiteral("pagination"), pagination);
        c->setStash(QStringLiteral("posts_count"), rows);
        c->setStash(QStringLiteral("search"), searchText);
    } else {
        c->response()->redirect(c->uriFor(CActionFor(QStringLiteral("notFound"))));
        return;
    }

    query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("SELECT uuid, title, raw, short, language, ip_address, user_agent, private, expires, created_at "
                               "FROM notes "
                               "WHERE private == 0 "
                               "AND (title LIKE '%'||:search||'%' "
                               "OR raw LIKE '%'||:search||'%') "
                               "ORDER BY id DESC "
                               "LIMIT :limit OFFSET :offset"),
                QStringLiteral("pastelyst"));
    query.bindValue(QStringLiteral(":search"), searchText);
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

            const Grantlee::SafeString shortHtml(obj.value(QStringLiteral("short")).toString(), true);
            obj.insert(QStringLiteral("short"), shortHtml);

            *it = obj;
            ++it;
        }
        c->setStash(QStringLiteral("notes"), list);
    }
}

void Root::notFound(Context *c)
{
    c->setStash(QStringLiteral("template"), QStringLiteral("404.html"));
    c->response()->setStatus(404);
}

void Root::cleanup()
{
    QSqlQuery query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("DELETE FROM notes WHERE expires > 0 AND strftime('%s','now') - strftime('%s', created_at) > expires"),
                QStringLiteral("pastelyst"));
    if (Q_UNLIKELY(!query.exec())) {
        qWarning() << "Failed to cleanup database" << query.lastError().databaseText();
    }
}

void Root::Auto(Context *)
{
    if (m_cleaupTimer.hasExpired(60 * 1000)) {
        cleanup();
        m_cleaupTimer.restart();
    }
}

bool Root::preFork(Application *app)
{
    m_socialMediaButtons = app->config(QStringLiteral("social"), true).toBool();
    m_downloadButton = app->config(QStringLiteral("download"), true).toBool();
    m_clipboardButton = app->config(QStringLiteral("clipboard"), true).toBool();

    return true;
}
