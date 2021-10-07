#ifndef BAILISQL_H
#define BAILISQL_H

#include <QtCore>
#include <QtSql>

namespace BailiSoft {

extern QStringList sqliteInitSqls(const QString &bookName, const bool forImport);
extern QString sqlInitBook(const QString &bookFilePath, const QString &bookName);

extern QVariant readValueFromSqliteFile(const QString &sql, const QString &sqliteFile = QString());
extern QString setValueToSqliteFile(const QStringList &sqls, const QString &sqliteFile = QString());
extern QStringList getExistsFieldsOfTable(const QString &table, QSqlDatabase &db);

}

#endif // BAILISQL_H
