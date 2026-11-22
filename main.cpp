#include "stdafx.h"
#include "Inkly.h"
#include "Fonts.h"
#include <QtWidgets/QApplication>
#include <QLocalServer>
#include <QLocalSocket>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":Inkly/Inkly.ico"));
    QCoreApplication::setOrganizationName("Inkly");
    QCoreApplication::setOrganizationDomain("kompanakdeveloperptero.pl");
    QCoreApplication::setApplicationName("Inkly");

    const QString arg = app.arguments().value(1);        // ścieżka pliku albo pusto
    const QString serverName = "Inkly-" + qEnvironmentVariable("USERNAME");

    // czy już działa inna kopia? Jeśli tak, przekaż jej ścieżkę i zakończ
    QLocalSocket probe;
    probe.connectToServer(serverName);
    if (probe.waitForConnected(300)) {
        probe.write(arg.toUtf8());
        probe.waitForBytesWritten(1000);
        return 0;
    }

    QLocalServer::removeServer(serverName);
    QLocalServer server;
    server.listen(serverName);

    loadUserFonts();
    Inkly window;
    window.show();

    QObject::connect(&server, &QLocalServer::newConnection, &window, [&server, &window] {
        QLocalSocket* c = server.nextPendingConnection();
        QObject::connect(c, &QLocalSocket::disconnected, c, &QObject::deleteLater);
        QObject::connect(c, &QLocalSocket::readyRead, &window, [c, &window] {
            const QString path = QString::fromUtf8(c->readAll());
            window.bringToFront();
            if (!path.isEmpty())
                window.openExternal(path);
            });
        });

    if (!arg.isEmpty())
        QTimer::singleShot(0, &window, [&window, arg] { window.openExternal(arg); });

    return app.exec();
}