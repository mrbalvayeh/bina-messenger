#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDebug>
#include <QProcess>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject{parent}
{
    // Initialize the PostgreSQL connection
    // Make sure Qt has PostgreSQL driver: "QPSQL"
    db_ = QSqlDatabase::addDatabase("QPSQL");
    db_.setHostName("127.0.0.1");
    db_.setPort(5432);
    db_.setDatabaseName("your_db_name");
    db_.setUserName("postgres");
    db_.setPassword("secret");

    if (!db_.open()) {
        qWarning() << "Failed to connect to database:" << db_.lastError().text();
    } else {
        qDebug() << "Connected to PostgreSQL successfully!";
    }
}
bool DatabaseManager::initializeDB() {
    // 1) Ensure Postgres is running
    if (!ensurePostgresRunning()) {
        qWarning() << "Could not start/find PostgreSQL service.";
        return false;
    }

    // 2) Create DB if missing
    if (!createDatabaseIfNotExists()) {
        qWarning() << "Failed to create/connect to database:" << m_mainDBName;
        return false;
    }

    // 3) Create or update schema (tables, admin user, etc.)
    if (!setupSchema()) {
        qWarning() << "Failed to setup schema for database:" << m_mainDBName;
        return false;
    }

    // 4) (Optional) Now open a QSqlDatabase connection for day-to-day queries
    //    e.g. addDatabase("QPSQL", "app_connection") ... connect to bina_messenger ...
    //    Then your authenticate() method can reuse that connection.

    return true;
}
bool DatabaseManager::ensurePostgresRunning() {
    // Quick test: connect to default "postgres" DB.
    {
        QSqlDatabase testDb = QSqlDatabase::addDatabase("QPSQL", "test_connection");
        testDb.setHostName(m_host);
        testDb.setPort(m_port);
        testDb.setDatabaseName("postgres");
        testDb.setUserName(m_adminUser);
        testDb.setPassword(m_adminPass);

        if (testDb.open()) {
            qDebug() << "Postgres is already running.";
            testDb.close();
            QSqlDatabase::removeDatabase("test_connection");
            return true;
        }
        testDb.close();
        QSqlDatabase::removeDatabase("test_connection");
    }

    // Not running, let's attempt to start it (requires sudo privileges)
    qDebug() << "Attempting to start Postgres service via systemctl...";
    QProcess process;
    process.start("sudo", QStringList() << "systemctl" << "start" << "postgresql");
    process.waitForFinished(5000);

    // Retest
    {
        QSqlDatabase testDb = QSqlDatabase::addDatabase("QPSQL", "test_connection2");
        testDb.setHostName(m_host);
        testDb.setPort(m_port);
        testDb.setDatabaseName("postgres");
        testDb.setUserName(m_adminUser);
        testDb.setPassword(m_adminPass);

        if (testDb.open()) {
            qDebug() << "Successfully started Postgres service.";
            testDb.close();
            QSqlDatabase::removeDatabase("test_connection2");
            return true;
        }
        qWarning() << "Still cannot connect after starting service:" << testDb.lastError().text();
        testDb.close();
        QSqlDatabase::removeDatabase("test_connection2");
    }

    return false;
}
bool DatabaseManager::createDatabaseIfNotExists() {
    // 1) Check if bina_messenger DB is present
    QString sqlCheck = QString("SELECT 1 FROM pg_database WHERE datname='%1';")
            .arg(m_mainDBName);

    QString errorMsg;
    if (!execSql("postgres", sqlCheck, &errorMsg)) {
        qWarning() << "Error checking database existence:" << errorMsg;
        return false;
    }

    // 2) Evaluate the query result
    {
        QSqlDatabase db = QSqlDatabase::database("postgres_temp");
        QSqlQuery query(db);
        if (!query.exec(sqlCheck)) {
            qWarning() << "Failed to check db existence:" << query.lastError().text();
            return false;
        }
        if (!query.next()) {
            // No row => DB doesn't exist => create it
            QString createSql = QString("CREATE DATABASE %1;").arg(m_mainDBName);
            if (!execSql("postgres", createSql, &errorMsg)) {
                qWarning() << "Failed to create DB:" << errorMsg;
                return false;
            }
            qDebug() << "Database" << m_mainDBName << "created successfully.";
        } else {
            qDebug() << "Database" << m_mainDBName << "already exists.";
        }
    }

    return true;
}
bool DatabaseManager::setupSchema() {
    // 1) Check if 'users' table exists
    QString checkTableSql = R"(
        SELECT 1
        FROM information_schema.tables
        WHERE table_schema='public'
          AND table_name='users';
    )";

    QString errorMsg;
    if (!execSql(m_mainDBName, checkTableSql, &errorMsg)) {
        qWarning() << "Error checking users table existence:" << errorMsg;
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(m_mainDBName + "_temp");
    QSqlQuery query(db);
    if (!query.exec(checkTableSql)) {
        qWarning() << "Error retrieving table existence:" << query.lastError().text();
        return false;
    }

    bool usersTableExists = query.next();

    // 2) If missing, create it
    if (!usersTableExists) {
        QString createSql = R"(
            CREATE TABLE users (
                id SERIAL PRIMARY KEY,
                username TEXT UNIQUE NOT NULL,
                password TEXT NOT NULL,
                is_admin BOOLEAN NOT NULL DEFAULT false
            );
        )";
        if (!execSql(m_mainDBName, createSql, &errorMsg)) {
            qWarning() << "Failed to create users table:" << errorMsg;
            return false;
        }
        qDebug() << "'users' table created.";
    }

    // 3) Insert an admin user if not present
    //    crypt(...) assumes you have the pgcrypto extension. Otherwise store hashed passwords in another way.
    QString insertAdminSql = R"(
        INSERT INTO users (username, password, is_admin)
        VALUES ('admin', crypt('adminpass', gen_salt('bf')), true)
        ON CONFLICT (username) DO NOTHING;
    )";
    if (!execSql(m_mainDBName, insertAdminSql, &errorMsg)) {
        qWarning() << "Failed to insert admin user:" << errorMsg;
        return false;
    }
    qDebug() << "Admin user ensured.";

    return true;
}
bool DatabaseManager::execSql(const QString &dbName, const QString &sql, QString *errorOut)
{
    // Unique connection name
    QString connName = dbName + "_temp";
    if (QSqlDatabase::contains(connName)) {
        QSqlDatabase::removeDatabase(connName);
    }

    // Add & open
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", connName);
    db.setHostName(m_host);
    db.setPort(m_port);
    db.setDatabaseName(dbName);
    db.setUserName(m_adminUser);
    db.setPassword(m_adminPass);

    if (!db.open()) {
        if (errorOut)
            *errorOut = db.lastError().text();
        db.close();
        return false;
    }

    // Exec
    QSqlQuery query(db);
    if (!query.exec(sql)) {
        if (errorOut)
            *errorOut = query.lastError().text();
        db.close();
        return false;
    }

    // Done
    return true;
}



// This method is exposed to QML via Q_INVOKABLE
bool DatabaseManager::authenticate(const QString &username, const QString &password)
{
    // For day-to-day app usage, connect to bina_messenger with a normal user
    // or keep using the same admin credentials if you like (not recommended in production)

    QSqlDatabase db = QSqlDatabase::database("app_connection");
    if (!db.isOpen()) {
        // Or open it here, or handle an error
        qWarning() << "DB not open in authenticate()!";
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT COUNT(*)
        FROM users
        WHERE username = :user
          AND password = crypt(:pass, password)
    )");
    query.bindValue(":user", username);
    query.bindValue(":pass", password);

    if (!query.exec()) {
        qWarning() << "Auth query error:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        int count = query.value(0).toInt();
        return (count == 1);
    }

    return false;
}
