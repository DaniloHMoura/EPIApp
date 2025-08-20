#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariant>

class DatabaseManager {
public:
    explicit DatabaseManager(const QString &db_name = "epi.db");
    ~DatabaseManager();
    bool executeQuery(const QString &queryStr, const QVariantList &params = QVariantList(), bool fetch = false, QVariantList *result = nullptr);

private:
    void initDatabase();
    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
