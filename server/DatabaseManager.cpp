#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent)
        : QObject(parent)
{
    // Optionally open connection right away
    openConnection();
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}
bool DatabaseManager::ensurePostgresRunning()
{
    int exitCode = std::system("pg_isready > /dev/null 2>&1");
    if (exitCode == 0) {
        return true; // already running
    }

    qInfo() << "PostgreSQL not running. Attempting to start it...";
    int startCode = std::system("sudo systemctl start postgresql");
    return (startCode == 0);
}
bool DatabaseManager::ensureDatabaseExists()
{
    QSqlDatabase checkDb = QSqlDatabase::addDatabase("QPSQL", "postgres_connection");
    checkDb.setHostName(m_host);
    checkDb.setPort(m_port);
    checkDb.setDatabaseName("postgres"); // connect to default db
    checkDb.setUserName(m_user);
    checkDb.setPassword(m_password);

    if (!checkDb.open()) {
        qWarning() << "Failed to connect to 'postgres' DB:" << checkDb.lastError().text();
        return false;
    }

    QSqlQuery checkQuery(checkDb);
    checkQuery.prepare("SELECT 1 FROM pg_database WHERE datname = :dbname");
    checkQuery.bindValue(":dbname", m_dbName);
    if (!checkQuery.exec()) {
        qWarning() << "DB existence check failed:" << checkQuery.lastError().text();
        return false;
    }

    if (checkQuery.next()) {
        return true; // DB already exists
    }

    qInfo() << "Database" << m_dbName << "does not exist. Creating it...";

    QSqlQuery createQuery(checkDb);
    if (!createQuery.exec(QString("CREATE DATABASE %1").arg(m_dbName))) {
        qWarning() << "Failed to create database:" << createQuery.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::initializeDB()
{
    if (!ensurePostgresRunning()) {
        qWarning() << "PostgreSQL service not running and couldn't start it.";
        return false;
    }

    if (!ensureDatabaseExists()) {
        qWarning() << "Failed to create/check target database.";
        return false;
    }

    if (!openConnection()) {
        qWarning() << "Failed to open PostgreSQL connection.";
        return false;
    }

    return setupSchema();
}

bool DatabaseManager::openConnection()
{
    if (m_db.isOpen()) {
        return true; // already open
    }

    // Using default connection name "app_connection" or choose your own
    if (!QSqlDatabase::contains("app_connection")) {
        m_db = QSqlDatabase::addDatabase("QPSQL", "app_connection");
    } else {
        m_db = QSqlDatabase::database("app_connection");
    }

    m_db.setHostName(m_host);
    m_db.setPort(m_port);
    m_db.setDatabaseName(m_dbName);
    m_db.setUserName(m_user);
    m_db.setPassword(m_password);

    if (!m_db.open()) {
        qWarning() << "Unable to open DB:" << m_db.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::setupSchema()
{
    QSqlQuery query(m_db);
    // 1) Create table if missing
    QString createUsers = R"(
        CREATE TABLE IF NOT EXISTS users (
            id SERIAL PRIMARY KEY,
            username TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL,
            is_admin BOOLEAN NOT NULL DEFAULT false
        )
    )";
    if (!query.exec(createUsers)) {
        qWarning() << "Failed to create 'users' table:"
                   << query.lastError().text();
        return false;
    }

    // 2) Ensure at least one admin if needed
    //    (Optional) e.g. insert default admin user
    QString insertAdmin = R"(
        INSERT INTO users (username, password, is_admin)
        VALUES ('admin', 'admin', true)
        ON CONFLICT (username) DO NOTHING
    )";
    if (!query.exec(insertAdmin)) {
        qWarning() << "Failed to ensure admin user:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::createUser(const QString &username, const QString &password, bool isAdmin)
{
    if (!m_db.isOpen()) {
        qWarning() << "DB not open in createUser!";
        return false;
    }
    // Check 5-user limit
    {
        QSqlQuery countQuery(m_db);
        if (!countQuery.exec("SELECT COUNT(*) FROM users")) {
            qWarning() << "Failed to count users:" << countQuery.lastError().text();
            return false;
        }
        if (countQuery.next()) {
            int userCount = countQuery.value(0).toInt();
            if (userCount >= 5) {
                qWarning() << "User limit (5) reached. Cannot create new user.";
                return false;
            }
        }
    }

    // Insert new user
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO users (username, password, is_admin)
        VALUES (:user, :pass, :admin)
        ON CONFLICT (username) DO NOTHING
    )");
    query.bindValue(":user", username);
    query.bindValue(":pass", password);    // For real usage, store hashed
    query.bindValue(":admin", isAdmin);

    if (!query.exec()) {
        qWarning() << "Insert user failed:" << query.lastError().text();
        return false;
    }
    if (query.numRowsAffected() < 1) {
        qWarning() << "No user inserted (duplicate username?).";
        return false;
    }
    return true;
}

QVariantList DatabaseManager::getAllUsers()
{
    QVariantList result;
    if (!m_db.isOpen()) {
        qWarning() << "DB not open in getAllUsers!";
        return result;
    }

    QSqlQuery query(m_db);
    if (!query.exec("SELECT username, is_admin FROM users ORDER BY id ASC")) {
        qWarning() << "Failed to select from users:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        QVariantMap map;
        map["username"] = query.value(0).toString();
        map["is_admin"] = query.value(1).toBool();
        result.append(map);
    }
    return result;
}

bool DatabaseManager::updateUser(const QString &oldUsername, const QString &newUsername, const QString &newPassword)
{
    if (!m_db.isOpen()) {
        qWarning() << "DB not open in updateUser!";
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE users
        SET username = :newUser,
            password = :newPass
        WHERE username = :oldUser
    )");
    query.bindValue(":newUser", newUsername);
    query.bindValue(":newPass", newPassword); // again, should be hashed
    query.bindValue(":oldUser", oldUsername);

    if (!query.exec()) {
        qWarning() << "Failed to update user:" << query.lastError().text();
        return false;
    }
    if (query.numRowsAffected() < 1) {
        qWarning() << "No user updated. Possibly oldUsername not found.";
        return false;
    }
    return true;
}

bool DatabaseManager::deleteUser(const QString &username)
{
    if (!m_db.isOpen()) {
        qWarning() << "DB not open in deleteUser!";
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM users WHERE username = :u");
    query.bindValue(":u", username);

    if (!query.exec()) {
        qWarning() << "Failed to delete user:" << query.lastError().text();
        return false;
    }
    return (query.numRowsAffected() > 0);
}

bool DatabaseManager::authenticate(const QString &username, const QString &password)
{
    if (!m_db.isOpen()) {
        qWarning() << "DB not open in authenticate!";
        return false;
    }

    QSqlQuery query(m_db);
    // For real usage, store hashed passwords, compare hashed input with stored hash
    query.prepare(R"(
        SELECT COUNT(*)
        FROM users
        WHERE username = :user
          AND password = :pass
    )");
    query.bindValue(":user", username);
    query.bindValue(":pass", password);

    if (!query.exec()) {
        qWarning() << "Auth query failed:" << query.lastError().text();
        return false;
    }
    if (query.next()) {
        return (query.value(0).toInt() == 1);
    }
    return false;
}
