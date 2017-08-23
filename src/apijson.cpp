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
#include "apijson.h"
#include "root.h"

#include "htmlhighlighter.h"

#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Utils/Pagination>
#include <Cutelyst/Plugins/Authentication/credentialpassword.h>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QCoreApplication>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QDebug>

using namespace Cutelyst;

ApiJson::ApiJson(HtmlHighlighter *htmlHighlighter, QObject *parent)
    : Controller(parent)
    , m_htmlHighlighter(htmlHighlighter)
{
}

void ApiJson::create(Context *c)
{
    if (!c->request()->isPost()) {
        return;
    }

    QJsonObject result;

    const ParamsMultiMap params = c->request()->bodyParameters();
    QString uuid;
    if (Root::createNote(c, m_htmlHighlighter, params, uuid)) {
        result.insert(QStringLiteral("id"), uuid);
    } else {
        result.insert(QStringLiteral("error"), uuid);
    }

    QJsonObject obj;
    obj.insert(QStringLiteral("result"), result);

    c->response()->setJsonBody(QJsonDocument(obj));
}

void ApiJson::show(Context *c, const QStringList &args)
{
    if (args.size() < 1) {
        return;
    }

    const QString uuid = args.first();
    QString userPassword;
    if (args.size() > 1) {
        userPassword = args.at(1);
    }

    QSqlQuery query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("SELECT title, raw, language, created_at, password "
                               "FROM notes "
                               "WHERE uuid = :uuid"),
                QStringLiteral("sticklyst"));
    query.bindValue(QStringLiteral(":uuid"), uuid);

    QJsonObject result;
    if (query.exec() && query.next()) {
        const QString password = query.value(QStringLiteral("password")).toString();
        if (!password.isEmpty()) {
            if (!CredentialPassword::validatePassword(userPassword.toUtf8(), password.toUtf8())) {
                return;
            }
        }

        result.insert(QStringLiteral("id"), uuid);
        result.insert(QStringLiteral("language"), query.value(QStringLiteral("language")).toString());
        result.insert(QStringLiteral("data"), query.value(QStringLiteral("raw")).toString());
        const QString title = query.value(QStringLiteral("title")).toString();
        if (title.isEmpty()) {
            result.insert(QStringLiteral("title"), QJsonValue());
        } else {
            result.insert(QStringLiteral("title"), title);
        }

        QDateTime dt = QDateTime::fromString(query.value(QStringLiteral("created_at")).toString(), Qt::ISODate);
        dt.setTimeSpec(Qt::LocalTime);
        result.insert(QStringLiteral("timestamp"), dt.toMSecsSinceEpoch() / 1000);
    } else {
        result.insert(QStringLiteral("error"), QStringLiteral("err_not_found"));
    }

    QJsonObject obj;
    obj.insert(QStringLiteral("result"), result);

    c->response()->setJsonBody(QJsonDocument(obj));
}

void ApiJson::list(Context *c, const QString &page)
{
    const int notesPerPage = 15;
    int offset;

    QSqlQuery query;
    query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("SELECT count(*) FROM notes WHERE private == 0"),
                QStringLiteral("sticklyst"));
    int rows;
    if (Q_LIKELY(query.exec() && query.next())) {
        rows = query.value(0).toInt();
        Pagination pagination(rows,
                              notesPerPage,
                              page.toInt());
        offset = pagination.offset();
    } else {
        return;
    }

    QJsonArray pastes;
    query = CPreparedSqlQueryThreadForDB(
                QStringLiteral("SELECT uuid FROM notes "
                               "WHERE private == 0 "
                               "ORDER BY id DESC "
                               "LIMIT :limit OFFSET :offset"),
                QStringLiteral("sticklyst"));
    query.bindValue(QStringLiteral(":limit"), notesPerPage);
    query.bindValue(QStringLiteral(":offset"), offset);
    if (query.exec()) {
        while (query.next()) {
            pastes.push_back(query.value(0).toString());
        }
    }

    QJsonObject result;
    result.insert(QStringLiteral("pastes"), pastes);
    result.insert(QStringLiteral("count"), notesPerPage);
    result.insert(QStringLiteral("pages"), rows / notesPerPage);

    QJsonObject obj;
    obj.insert(QStringLiteral("result"), result);

    c->response()->setJsonBody(QJsonDocument(obj));
}

void ApiJson::parameterExpire(Context *c)
{
    QJsonArray values;
    values.append(1800);
    values.append(21600);
    values.append(86400);
    values.append(604800);
    values.append(2592000);
    values.append(31536000);
    values.append(0);

    QJsonObject result;
    result.insert(QStringLiteral("values"), values);

    QJsonObject obj;
    obj.insert(QStringLiteral("result"), result);

    c->response()->setJsonBody(QJsonDocument(obj));
}

void ApiJson::parameterLanguage(Context *c)
{
    QJsonArray values;

    const QStringList languages = m_htmlHighlighter->languages();
    for (const QString &value : languages) {
        values.push_back(value);
    }

    QJsonObject result;
    result.insert(QStringLiteral("values"), values);

    QJsonObject obj;
    obj.insert(QStringLiteral("result"), result);

    c->response()->setJsonBody(QJsonDocument(obj));
}

void ApiJson::parameterVersion(Context *c)
{
    QJsonArray values;
    values.append(QCoreApplication::applicationVersion());

    QJsonObject result;
    result.insert(QStringLiteral("values"), values);

    QJsonObject obj;
    obj.insert(QStringLiteral("result"), result);

    c->response()->setJsonBody(QJsonDocument(obj));
}

void ApiJson::parameterTheme(Context *c)
{
    QJsonArray values;
    values.append(QStringLiteral("Default"));

    QJsonObject result;
    result.insert(QStringLiteral("values"), values);

    QJsonObject obj;
    obj.insert(QStringLiteral("result"), result);

    c->response()->setJsonBody(QJsonDocument(obj));
}
