#include "ZmqServer.h"
#include "messages.pb.h"    // from your shared/ dir, compiled by protoc
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include "DatabaseManager.h"
ZmqServer::ZmqServer(DatabaseManager &dbManager)
        : m_db(dbManager),
          m_context(1),                 // 1 = one I/O thread
          m_repSocket(m_context, zmq::socket_type::rep)
{
}

void ZmqServer::run(const std::string &bindAddress)
{
    // 1) Bind
    m_repSocket.bind(bindAddress);

    qDebug() << "ZMQ Server listening on" << QString::fromStdString(bindAddress);

    // 2) Main loop
    while (true) {
        // Receive request
        zmq::message_t requestMsg;
        auto result = m_repSocket.recv(requestMsg, zmq::recv_flags::none);
        if (!result.has_value()) {
            qWarning() << "Failed to receive message from client.";
            continue;
        }

        // Parse request
        myapp::MyRequest req;
        if (!req.ParseFromArray(requestMsg.data(), requestMsg.size())) {
            // If parse fails, respond with an error
            myapp::MyResponse errorResp;
            errorResp.set_success(false);
            errorResp.set_error("Failed to parse protobuf request.");

            std::string errorData = errorResp.SerializeAsString();
            zmq::message_t errorMsg(errorData.size());
            memcpy(errorMsg.data(), errorData.data(), errorData.size());
            m_repSocket.send(errorMsg, zmq::send_flags::none);
            continue;
        }

        // Construct response
        myapp::MyResponse resp;
        resp.set_success(true); // default

        // Switch on the command
        switch (req.cmd()) {
            case myapp::MyRequest::CREATE_USER: {
                bool ok = m_db.createUser(
                        QString::fromStdString(req.username()),
                        QString::fromStdString(req.password()),
                        req.is_admin()
                );
                if (!ok) {
                    resp.set_success(false);
                    resp.set_error("createUser failed; maybe limit reached or username exists.");
                }
                break;
            }
            case myapp::MyRequest::DELETE_USER: {
                bool ok = m_db.deleteUser(QString::fromStdString(req.username()));
                if (!ok) {
                    resp.set_success(false);
                    resp.set_error("deleteUser failed; user not found?");
                }
                break;
            }
            case myapp::MyRequest::UPDATE_USER: {
                bool ok = m_db.updateUser(
                        QString::fromStdString(req.username()),
                        QString::fromStdString(req.new_username()),
                        QString::fromStdString(req.password())
                );
                if (!ok) {
                    resp.set_success(false);
                    resp.set_error("updateUser failed.");
                }
                break;
            }
            case myapp::MyRequest::LIST_USERS: {
                // getAllUsers() returns QVariantList of QVariantMap
                QVariantList userList = m_db.getAllUsers();
                for (auto &elem : userList) {
                    QVariantMap map = elem.toMap();
                    resp.add_users(map["username"].toString().toStdString());
                }
                break;
            }
            case myapp::MyRequest::AUTHENTICATE: {
                QSqlQuery query(QSqlDatabase::database("app_connection"));
                query.prepare(R"(
        SELECT is_admin FROM users WHERE username = :u AND password = :p
    )");
                query.bindValue(":u", QString::fromStdString(req.username()));
                query.bindValue(":p", QString::fromStdString(req.password()));

                if (query.exec() && query.next()) {
                    resp.set_success(true);
                    resp.set_is_admin(query.value(0).toBool());
                } else {
                    resp.set_success(false);
                    resp.set_error("Invalid credentials");
                }
                break;
            }

            default:
                resp.set_success(false);
                resp.set_error("Unknown cmd in request.");
                break;
        }

        // Send response
        std::string outData = resp.SerializeAsString();
        zmq::message_t outMsg(outData.size());
        memcpy(outMsg.data(), outData.data(), outData.size());
        m_repSocket.send(outMsg, zmq::send_flags::none);
    }
}
