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

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QUuid>

#include <QDebug>

using namespace Cutelyst;

Root::Root(QObject *parent) : Controller(parent),
    m_htmlHighlighter(new HtmlHighlighter)
{
    qDebug() << QUuid::createUuid().toRfc4122().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    qDebug() << QUuid::createUuid().toRfc4122().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    qDebug() << QUuid::createUuid().toRfc4122().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
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
    qDebug() << uuid << "uuid";
    QSqlQuery query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("SELECT title, raw, html, definition, ip_address, user_agent, private, expires_at, created_at "
                               "FROM notes "
                               "WHERE uuid = :uuid"),
                QStringLiteral("sticklyst"));
    query.bindValue(QStringLiteral(":uuid"), uuid);
    if (query.exec()) {
        qDebug() << "EXEC";
        c->setStash(QStringLiteral("note"), Sql::queryToHashObject(query));
    }
}

void Root::create(Context *c)
{
    if (!c->request()->isPost()) {
        c->res()->redirect(c->uriFor(actionFor(QStringLiteral("index"))));
        return;
    }

    const ParamsMultiMap params = c->request()->bodyParams();
    const QString title = params.value(QStringLiteral("title"));
    const QString definition = params.value(QStringLiteral("language"));
    const QString expire = params.value(QStringLiteral("expire"));
    const QString priv = params.value(QStringLiteral("private"));

    QString data = params.value(QStringLiteral("data"));
    const QString dataHighlighted = m_htmlHighlighter->highlightString(definition, QStringLiteral("Default"), &data);

    const QString uuid = QString::fromLatin1(
                QUuid::createUuid().toRfc4122()
                .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals).left(6));

    QSqlQuery query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("INSERT INTO notes "
                               "(uuid, title, raw, html, definition, ip_address, user_agent, private, expires_at, created_at) "
                               "VALUES "
                               "(:uuid, :title, :raw, :html, :definition, :ip_address, :user_agent, :private, :expires_at, :created_at)"),
                QStringLiteral("sticklyst"));
    query.bindValue(QStringLiteral(":uuid"), uuid);
    query.bindValue(QStringLiteral(":title"), title);
    query.bindValue(QStringLiteral(":raw"), data);
    query.bindValue(QStringLiteral(":html"), dataHighlighted);
    query.bindValue(QStringLiteral(":definition"), title);
    query.bindValue(QStringLiteral(":ip_address"), c->request()->addressString());
    query.bindValue(QStringLiteral(":user_agent"), c->request()->userAgent());
    query.bindValue(QStringLiteral(":private"), !priv.isEmpty());
    query.bindValue(QStringLiteral(":expires_at"), 3600);
    query.bindValue(QStringLiteral(":created_at"), QDateTime::currentDateTimeUtc());
    if (query.exec() && query.numRowsAffected() == 1) {

    }
    qDebug() << "Creating Paste" << uuid << query.lastError().databaseText();

    c->res()->redirect(c->uriFor(CActionFor(QStringLiteral("item")), QStringList{ uuid }));
}

void Root::defaultPage(Context *c)
{
    c->response()->body() = "Page not found!";
    c->response()->setStatus(404);
}

