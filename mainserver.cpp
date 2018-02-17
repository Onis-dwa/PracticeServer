#include "mainserver.h"
#include "ui_mainserver.h"
#include "ws.h"
#include "pc.h"
#include "settings.h"

#include <QMessageBox>
#include <QDateTime>

// public
MainServer::MainServer(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainServer)
{
	uint port = loadSettings();
	mTcpServer = new QTcpServer(this);
	if (!mTcpServer->listen(QHostAddress::AnyIPv4, port)){
		QMessageBox::critical(0, "Server error", "Unable to srart the server:" + mTcpServer->errorString());

		mTcpServer->close();
		logfile("Server is not started");
		return;
	}
	connect(mTcpServer, SIGNAL(newConnection()), SLOT(slotNewConnection()));
	logfile("Server started");


	ui->setupUi(this);
	mTcpSocket = NULL;
	iNextBlockSize = 0;
	winset = new Settings(this);
	connect(winset, SIGNAL(settingsChanged(quint16, QString, QString)),
					SLOT(settingsChanged(quint16, QString, QString)));


	ws = new WS(this, ui->CW);
	layout = new QGridLayout;
	layout->addWidget(ws);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->setSizeConstraint(layout->SetDefaultConstraint);
	ui->CW->setLayout(layout);

	connect(ui->menuBar->addAction("Настройки"), SIGNAL(triggered(bool)), SLOT(menuSettings(bool)));
	QAction* btn = ui->menuBar->addAction("Добавить PC");
	connect(btn, &QAction::toggled, ws, &WS::slotToggleBtn);
	connect(ws, &WS::SetChecked, btn, &QAction::setChecked);
	btn->setCheckable(true);
	btn->setChecked(false);

	pclist = new QList<pc*>();
	loadpcs();
	connect(ws, &WS::newPC, this, &MainServer::newPC);
}
MainServer::~MainServer()
{
	logfile("Server closed");

	delete ui;
	delete mTcpServer;

	if (mTcpSocket != NULL)
		delete mTcpSocket;

	delete flog;
	delete fsettings;
	delete fpcs;

	for (int i = 0; i < pclist->count(); i++) {
		delete pclist->at(i);
	}
	delete pclist;
}
void MainServer::logfile(const QString& str)
{
	if (!flog->open(QIODevice::Append | QIODevice::Text)) {
		QMessageBox::critical(0, "Log error", "Failed write to file");
		return;
	}

	QTextStream out(flog);
	out << QDateTime::currentDateTime().date().year() << "." <<
		   QDateTime::currentDateTime().date().month() << "." <<
		   QDateTime::currentDateTime().date().day() << "-" <<
		   QDateTime::currentDateTime().time().hour() << ":" <<
		   QDateTime::currentDateTime().time().minute() << ":" <<
		   QDateTime::currentDateTime().time().second() << "-" <<
		   str << "\n";
	flog->close();
}

// private
uint MainServer::loadSettings()
{
	uint port;
	QString strlog;
	QString strpcs;
	QStringList buf;

	fsettings = new QFile("settings.txt");
	if (fsettings->exists()) {
		if (fsettings->open(QIODevice::ReadOnly | QIODevice::Text)) {
			strlog = fsettings->readLine();
			buf = strlog.split(' ');

			port = buf.at(0).toUInt();
			strlog = buf.at(1);
			strpcs = buf.at(2);
		}
	}
	else {
		if (fsettings->open(QIODevice::WriteOnly | QIODevice::Text)) {
			QTextStream out (fsettings);
			out << "32094 log.txt pcs.txt";
		}
		port = 32094;
		strlog = "log.txt";
		strpcs = "pcs.txt";
	}

	flog = new QFile(strlog);
	fpcs = new QFile(strpcs);
	fsettings->close();

	return port;
}
void MainServer::loadpcs()
{
	if (!fpcs->exists()) {
		QMessageBox::critical(0, "PC's file error", "File does not exists");
		return;
	}
	if (!fpcs->open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::critical(0, "PC's file error", "Failed read to file");
		return;
	}

	QStringList buf;
	pcData* dt = new pcData();

	try {
		while (!fpcs->atEnd()) {
			buf = ((QString)fpcs->readLine()).split(' ');

			dt->x = buf.at(0).toInt();
			dt->y = buf.at(1).toInt();
			dt->MServ = this;
			dt->Name = buf.at(2);
			dt->IP = buf.at(3);

			pc* npc = ws->addPc(*dt);
			pclist->push_back(npc);
//			connect(npc, SIGNAL(SetChanged(QString, QString, QString)), this, SLOT(SetChanged(QString, QString, QString)));
		}
	}
	catch (...) {
		QMessageBox::critical(0, "PC's file error", "Failed is corrupt, error loading pc's");
	}
	fpcs->close();
}

// public slot
void MainServer::slotNewConnection()
{
	mTcpSocket = mTcpServer->nextPendingConnection();

	if (pclist->count() < 1) {
		mTcpSocket->close();
		return;
	}

	for (int i = 0; i < pclist->count(); i++) {
		qDebug() << "client" << mTcpSocket->peerAddress();
		qDebug() << "ip: " + pclist->at(i)->GetData()->IP;
		if (mTcpSocket->peerAddress().toString() == pclist->at(i)->GetData()->IP) {
			pclist->at(i)->Connect(mTcpSocket);

			return;
		}
	}
}
void MainServer::menuSettings(bool)
{
	if (!winset->isVisible())
	{
		winset->setData(mTcpServer->serverAddress().toString(),
						 mTcpServer->serverPort(),
						 flog->fileName(),
						 fpcs->fileName());
		winset->show();
	}
}
void MainServer::settingsChanged(quint16 port, QString nlf, QString npf)
{
	mTcpServer->close();
	mTcpServer = new QTcpServer(this);
	if (!mTcpServer->listen(QHostAddress::AnyIPv4, port)){
		QMessageBox::critical(0, "Server error", "Unable to change the port:" + mTcpServer->errorString());

		mTcpServer->close();
		logfile("Server stopped");
		this->close();
		return;
	}

	flog->setFileName(nlf);
	fpcs->setFileName(npf);

	if (fsettings->open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream out (fsettings);
		out << port << " " << nlf << " " <<npf;
		fsettings->close();
	}
	logfile("settings changed to: " + QString::number(port) + " " + nlf + " " + npf);
}
//void MainServer::SetChanged(QString name, QString ip, QString oldip)
//{
//	return;

//	if (!fpcs->open(QIODevice::ReadWrite | QIODevice::Text)) {
//		QMessageBox::critical(0, "Pc's error", "failed append to file");
//		return;
//	}

//	QString line;
//	QStringList buf;

//	while (!fpcs->atEnd()) {
//		line = fpcs->readLine();
//		buf = line.split(' ');

//		if (buf.at(3) == oldip) {
//			fpcs->seek(fpcs->pos() - line.length() - 1);

//			QTextStream out (fpcs);
//			out << buf.at(0) << " " <<
//				   buf.at(1) << " " <<
//				   name << " " <<
//				   ip << " \n";
//		}
//	}
//	fpcs->close();

//	logfile(name + " settings changed!");
//}
void MainServer::newPC(pcData& data)
{
	if (!fpcs->open(QIODevice::Append | QIODevice::Text)) {
		QMessageBox::critical(0, "PC's error", "Failed append to file");
		return;
	}

	QTextStream out(fpcs);
	out << data.x << ' '<<
		   data.y << ' ' <<
		   data.Name << ' ' <<
		   data.IP << " \n";
	fpcs->close();

	pc* npc = ws->addPc(data);
	pclist->push_back(npc);
//	connect(npc, SIGNAL(SetChanged(QString,QString)), this, SLOT(SetChanged(QString, QString)));
}
