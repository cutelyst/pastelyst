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
#include "pastelyst.h"

#include "root.h"
#include "apijson.h"
#include "htmlhighlighter.h"

#include <Cutelyst/Plugins/View/Cutelee/cuteleeview.h>
#include <Cutelyst/Plugins/Utils/Sql>
#include <Cutelyst/Plugins/Session/Session>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>
#include <QMutexLocker>

using namespace Cutelyst;

static QMutex mutex;

Pastelyst::Pastelyst(QObject *parent) : Application(parent)
{
}

Pastelyst::~Pastelyst()
{
}

bool Pastelyst::init()
{
    auto htmlHighlighter = new HtmlHighlighter;
    new Root(htmlHighlighter, this);
    new ApiJson(htmlHighlighter, this);

    bool production = config(QStringLiteral("production")).toBool();
    qDebug() << "Production" << production;

    m_dbPath = config(QStringLiteral("DatabasePath"),
                      QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QLatin1String("/pastelyst.sqlite")).toString();
    qDebug() << "Database" << m_dbPath;
    if (!QFile::exists(m_dbPath)) {
        if (!createDB()) {
            qDebug() << "Failed to create databese" << m_dbPath;
            return false;
        }
        QSqlDatabase::removeDatabase(QStringLiteral("db"));
    }

    auto view = new CuteleeView(this);
    view->setIncludePaths({ pathTo(QStringLiteral("root/src")) });
    view->setTemplateExtension(QStringLiteral(".html"));
    view->setWrapper(QStringLiteral("wrapper.html"));
    view->setCache(production);

    new Session(this);

    return true;
}

bool Pastelyst::postFork()
{
    QMutexLocker locker(&mutex);

    auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), Cutelyst::Sql::databaseNameThread(QStringLiteral("pastelyst")));
    db.setDatabaseName(m_dbPath);
    if (!db.open()) {
        qWarning() << "Failed to open database" << db.lastError().databaseText();
        return false;
    }

    qDebug() << "Database ready" << db.connectionName();

    Root::cleanup();

    return true;
}

bool Pastelyst::createDB()
{
    auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("db"));
    db.setDatabaseName(m_dbPath);
    if (!db.open()) {
        qWarning() << "Failed to open database" << db.lastError().databaseText();
        return false;
    }

    QSqlQuery query(db);
    qDebug() << "Creating database" << m_dbPath;

    bool ret = query.exec(QStringLiteral("PRAGMA journal_mode = WAL"));
    qDebug() << "PRAGMA journal_mode = WAL" << ret << query.lastError().databaseText();

    if (!query.exec(QStringLiteral("CREATE TABLE notes "
                                   "( id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT"
                                   ", uuid TEXT NOT NULL UNIQUE "
                                   ", title TEXT "
                                   ", raw TEXT "
                                   ", html TEXT "
                                   ", short TEXT "
                                   ", language TEXT "
                                   ", password TEXT "
                                   ", user_id INTEGER "
                                   ", ip_address TEXT "
                                   ", user_agent TEXT "
                                   ", private BOOL NOT NULL "
                                   ", expires INTEGER "
                                   ", created_at datetime NOT NULL "
                                   ")"))) {
        qCritical() << "Error creating database" << query.lastError().text();
        return false;
    }

    return true;
}
