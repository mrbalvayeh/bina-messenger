#include "ClientAPI.h"
#include "messages.pb.h"  // Generated from your shared/messages.proto
#include <QDebug>
//bool ClientAPI::authenticate(const QString &username, const QString &password)
//{
//    myapp::MyRequest req;
//    req.set_cmd(myapp::MyRequest::AUTHENTICATE);
//    req.set_username(username.toStdString());
//    req.set_password(password.toStdString());
//
//    std::string outData = req.SerializeAsString();
//    std::string inData;
//
//    if (!sendRequest(outData, inData))
//        return false;
//
//    myapp::MyResponse resp;
//    if (!resp.ParseFromString(inData)) {
//        qWarning() << "Failed to parse authenticate response";
//        return false;
//    }
//    if (!resp.success()) {
//        qWarning() << "Server error (authenticate):" << QString::fromStdString(resp.error());
//        return false;
//    }
//
//    return true;
//}
QVariantMap ClientAPI::authenticate(const QString &username, const QString &password)
{
    QVariantMap result;

    myapp::MyRequest req;
    req.set_cmd(myapp::MyRequest::AUTHENTICATE);
    req.set_username(username.toStdString());
    req.set_password(password.toStdString());

    std::string outData = req.SerializeAsString();
    std::string inData;

    if (!sendRequest(outData, inData)) {
        result["success"] = false;
        result["error"] = "Network error";
        return result;
    }

    myapp::MyResponse resp;
    if (!resp.ParseFromString(inData)) {
        result["success"] = false;
        result["error"] = "Invalid response format";
        return result;
    }

    result["success"] = resp.success();
    result["isAdmin"] = resp.is_admin();
    result["error"] = QString::fromStdString(resp.error());
    return result;
}

ClientAPI::ClientAPI(QObject *parent)
        : QObject(parent)
{
    // 1) Create ZeroMQ context & REQ socket (persistent)
    m_context = std::make_unique<zmq::context_t>(1);
    m_reqSocket = std::make_unique<zmq::socket_t>(*m_context, zmq::socket_type::req);

    // 2) Connect to your server’s endpoint
    //    Adjust if your server is on a different IP/port
    m_reqSocket->connect("tcp://127.0.0.1:5555");
}

bool ClientAPI::createUser(const QString &username, const QString &password, bool isAdmin)
{
    // Build protobuf request
    myapp::MyRequest req;
    req.set_cmd(myapp::MyRequest::CREATE_USER);
    req.set_username(username.toStdString());
    req.set_password(password.toStdString());
    req.set_is_admin(isAdmin);

    // Serialize
    std::string outData = req.SerializeAsString();
    std::string inData;

    // Send & receive
    if (!sendRequest(outData, inData))
        return false;

    // Parse response
    myapp::MyResponse resp;
    if (!resp.ParseFromString(inData)) {
        qWarning() << "Failed to parse createUser response";
        return false;
    }
    if (!resp.success()) {
        qWarning() << "Server error (createUser):"
                   << QString::fromStdString(resp.error());
        return false;
    }
    return true;
}

bool ClientAPI::deleteUser(const QString &username)
{
    myapp::MyRequest req;
    req.set_cmd(myapp::MyRequest::DELETE_USER);
    req.set_username(username.toStdString());

    std::string outData = req.SerializeAsString();
    std::string inData;
    if (!sendRequest(outData, inData))
        return false;

    myapp::MyResponse resp;
    if (!resp.ParseFromString(inData)) {
        qWarning() << "Failed to parse deleteUser response";
        return false;
    }
    if (!resp.success()) {
        qWarning() << "Server error (deleteUser):"
                   << QString::fromStdString(resp.error());
        return false;
    }
    return true;
}

bool ClientAPI::updateUser(const QString &oldUsername, const QString &newUsername, const QString &newPassword)
{
    myapp::MyRequest req;
    req.set_cmd(myapp::MyRequest::UPDATE_USER);
    req.set_username(oldUsername.toStdString());
    req.set_new_username(newUsername.toStdString());
    req.set_password(newPassword.toStdString());

    std::string outData = req.SerializeAsString();
    std::string inData;
    if (!sendRequest(outData, inData))
        return false;

    myapp::MyResponse resp;
    if (!resp.ParseFromString(inData)) {
        qWarning() << "Failed to parse updateUser response";
        return false;
    }
    if (!resp.success()) {
        qWarning() << "Server error (updateUser):"
                   << QString::fromStdString(resp.error());
        return false;
    }
    return true;
}

QVariantList ClientAPI::listUsers()
{
    QVariantList list;

    myapp::MyRequest req;
    req.set_cmd(myapp::MyRequest::LIST_USERS);

    std::string outData = req.SerializeAsString();
    std::string inData;
    if (!sendRequest(outData, inData))
        return list; // empty

    myapp::MyResponse resp;
    if (!resp.ParseFromString(inData)) {
        qWarning() << "Failed to parse listUsers response";
        return list;
    }
    if (!resp.success()) {
        qWarning() << "Server error (listUsers):"
                   << QString::fromStdString(resp.error());
        return list;
    }

    // Convert repeated `users` (strings) into a QVariantList of QVariantMap
    for (int i = 0; i < resp.users_size(); ++i) {
        QVariantMap map;
        map["username"] = QString::fromStdString(resp.users(i));
        list.append(map);
    }
    return list;
}

bool ClientAPI::sendRequest(const std::string &serializedRequest, std::string &serializedResponse)
{
    // Send
    zmq::message_t requestMsg(serializedRequest.size());
    memcpy(requestMsg.data(), serializedRequest.data(), serializedRequest.size());
    m_reqSocket->send(requestMsg, zmq::send_flags::none);

    // Receive
    zmq::message_t replyMsg;
    auto result = m_reqSocket->recv(replyMsg, zmq::recv_flags::none);
    if (!result.has_value()) {
        qWarning() << "No response from server or recv failed!";
        return false;
    }

    // Copy received data
    serializedResponse.assign(
            static_cast<const char*>(replyMsg.data()),
            replyMsg.size()
    );
    return true;
}
bool ClientAPI::createUser(const QString &username, const QString &password)
{
    myapp::MyRequest req;
    req.set_cmd(myapp::MyRequest::CREATE_USER);
    req.set_username(username.toStdString());
    req.set_password(password.toStdString());

    std::string outData = req.SerializeAsString();
    std::string inData;

    if (!sendRequest(outData, inData)) return false;

    myapp::MyResponse resp;
    if (!resp.ParseFromString(inData)) return false;

    return resp.success();
}

QVariantList ClientAPI::getAllUsers()
{
    myapp::MyRequest req;
    req.set_cmd(myapp::MyRequest::LIST_USERS);

    std::string outData = req.SerializeAsString();
    std::string inData;

    QVariantList users;

    if (!sendRequest(outData, inData)) return users;

    myapp::MyResponse resp;
    if (!resp.ParseFromString(inData) || !resp.success()) return users;

    for (const auto& user : resp.users()) {
        QVariantMap userMap;
        userMap["username"] = QString::fromStdString(user);
        userMap["is_admin"] = (user == "admin");
        users.append(userMap);
    }

    return users;
}


