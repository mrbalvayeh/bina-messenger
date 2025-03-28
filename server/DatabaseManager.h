#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QVariantList>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    bool ensurePostgresRunning();
    bool ensureDatabaseExists();

    // Initialize DB, create tables if not exist, etc.
    bool initializeDB();

    // CRUD
    bool createUser(const QString &username, const QString &password, bool isAdmin = false);
    QVariantList getAllUsers();
    bool updateUser(const QString &oldUsername, const QString &newUsername, const QString &newPassword);
    bool deleteUser(const QString &username);

    // Just an example from earlier if you want an auth check:
    bool authenticate(const QString &username, const QString &password);

private:
    QSqlDatabase m_db;
    QSqlDatabase db() const { return m_db; }
    // Connection details
    QString m_host      = "127.0.0.1";
    int     m_port      = 5432;
    QString m_dbName    = "bina_messenger";
    QString m_user      = "postgres";
    QString m_password  = "secret";

    // Helper to ensure our DB is open
    bool openConnection();

    // Helper to create/verify schema
    bool setupSchema();

    // If you need separate logic for ensuring Postgres is running, etc., put it here
};

#endif // DATABASEMANAGER_H
