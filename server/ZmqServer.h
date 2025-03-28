//
// Created by dev on 3/28/25.
//

#ifndef BINA_MESSENGER_ZMQSERVER_H
#define BINA_MESSENGER_ZMQSERVER_H

#include <zmq.hpp>
#include "DatabaseManager.h"
class ZmqServer {
public:
    // Pass a reference to your DatabaseManager
    ZmqServer(DatabaseManager &dbManager);

    // Start the server loop (blocking)
    void run(const std::string &bindAddress = "tcp://*:5555");

private:
    DatabaseManager &m_db;
    zmq::context_t m_context;   // keep a persistent context
    zmq::socket_t  m_repSocket; // REP socket
};



#endif //BINA_MESSENGER_ZMQSERVER_H
