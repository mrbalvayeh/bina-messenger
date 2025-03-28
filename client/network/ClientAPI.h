#ifndef BINA_CLIENTAPI_H
#define BINA_CLIENTAPI_H
#include <QObject>
#include <memory>
#include <zmq.hpp>
#include <QString>
#include <QVariantList>


/*!
 * \brief The ClientAPI class
 * Manages a persistent ZeroMQ connection to the server and provides
 * methods (createUser, deleteUser, etc.) that send Protobuf requests.
 */
class ClientAPI : public QObject
{
Q_OBJECT
public:
    struct AuthResult {
        bool success;
        bool isAdmin;
    };
    Q_INVOKABLE bool createUser(const QString &username, const QString &password);
    Q_INVOKABLE bool updateUser(const QString &oldUsername, const QString &newUsername, const QString &newPassword);
    Q_INVOKABLE QVariantList getAllUsers();
    Q_INVOKABLE QVariantMap authenticate(const QString &username, const QString &password);
//    Q_INVOKABLE bool authenticate(const QString &username, const QString &password);
    explicit ClientAPI(QObject *parent = nullptr);

    // Basic CRUD calls
    Q_INVOKABLE bool createUser(const QString &username, const QString &password, bool isAdmin);
    Q_INVOKABLE bool deleteUser(const QString &username);
//    Q_INVOKABLE bool updateUser(const QString &oldUsername, const QString &newUsername, const QString &newPassword);
    Q_INVOKABLE QVariantList listUsers();

private:
    // Persistent ZeroMQ objects
    std::unique_ptr<zmq::context_t> m_context;
    std::unique_ptr<zmq::socket_t>  m_reqSocket;

    // Helper to send serialized request & receive serialized response
    bool sendRequest(const std::string &serializedRequest, std::string &serializedResponse);
};
#endif //BINA_CLIENTAPI_H
