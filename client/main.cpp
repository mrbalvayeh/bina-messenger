#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "network/ClientAPI.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    ClientAPI clientAPI;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("clientAPI", &clientAPI);

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    engine.load(url);

    return app.exec();
}
