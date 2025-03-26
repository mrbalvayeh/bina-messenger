#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
//#include "databasemanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
//    DatabaseManager dbManager;
    QQmlApplicationEngine engine;
//    engine.rootContext()->setContextProperty("dbManager", &dbManager);
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    engine.load(url);

    return app.exec();
}
