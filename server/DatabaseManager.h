#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    bool initializeDB();
    Q_INVOKABLE bool authenticate(const QString &username, const QString &password);

signals:
private:
    QSqlDatabase db_;
    bool ensurePostgresRunning();
    bool createDatabaseIfNotExists();
    bool setupSchema();
    bool execSql(const QString &dbName, const QString &sql, QString *errorOut = nullptr);

    QString m_host = "localhost";
    int     m_port = 5432;

    QString m_adminUser  = "postgres";   // superuser or user with CREATE DB privileges
    QString m_adminPass  = "secret";     // or empty if using local trust/peer auth
    QString m_mainDBName = "bina_messenger";
};

#endif // DATABASEMANAGER_H
