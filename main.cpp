#include "mainserver.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	MainServer serv;
	serv.show();

	return app.exec();
}
