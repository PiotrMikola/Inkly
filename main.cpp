#include "stdafx.h"
#include "Inkly.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
	app.setWindowIcon(QIcon(":Inkly/Inkly.ico"));
	QCoreApplication::setOrganizationName("Inkly");
	QCoreApplication::setOrganizationDomain("kompanakdeveloperptero.pl");
	QCoreApplication::setApplicationName("Inkly");
    Inkly window;
    window.show();
    return app.exec();
}
