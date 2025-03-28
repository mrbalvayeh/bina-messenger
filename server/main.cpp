#include <QCoreApplication>
#include "DatabaseManager.h"
#include "ZmqServer.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    DatabaseManager dbManager;
    if (!dbManager.initializeDB()) {
        qWarning() << "Failed to initialize PostgreSQL. Exiting.";
        return -1;
    }

    // Start ZeroMQ server
    ZmqServer zmqServer(dbManager);
    zmqServer.run("tcp://*:5555"); // Blocks forever

    return app.exec(); // Actually, with the while(true) in run(), you might never get here
}
