#include "mainserver.h"
#include "ui_mainserver.h"
#include "ws.h"

#include <QMessageBox>
#include <QTime>
#include <QGridLayout>

MainServer::MainServer(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainServer)
{
	ui->setupUi(this);

	mTcpServer = new QTcpServer(this);
	if (!mTcpServer->listen(QHostAddress::LocalHost, 32094)){
		QMessageBox::critical(0, "Server error", "Unable to srart the server:" + mTcpServer->errorString());

		mTcpServer->close();
//		ui->listWidget->addItem("Server is not started");
		return;
	}

	connect(mTcpServer, &QTcpServer::newConnection, this, &MainServer::slotNewConnection);
//	ui->listWidget->addItem("Server started");

//	connect(ui->addpc, SIGNAL(clicked(bool)), SLOT(AddPC(bool)));
//	connect(ui->WS, &QWidget::mouseMoveEvent,this, &MainServer::MouseMove);
//	connect(,SIGNAL(),SLOT());
	iNextBlockSize = 0;

	WS* ws = new WS(this, ui->CW);
	QGridLayout* layout = new QGridLayout;
	layout->addWidget(ws);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->setSizeConstraint(layout->SetDefaultConstraint);

	ui->CW->setLayout(layout);
//	ui->CW->stackUnder(ws);
//	ws->stackUnder(ui->CW);
//	this->cursor().setShape(Qt::CrossCursor);
//	ui->CW->cursor().setShape(Qt::CrossCursor);
//	ws->setCursor(QCursor(Qt::CrossCursor));
}

MainServer::~MainServer()
{
	// логи
	delete ui;
}

void MainServer::slotNewConnection()
{
	QTcpSocket* pClientSocket = mTcpServer->nextPendingConnection();

	//connect(pClientSocket, &QTcpSocket::readyRead, this, &MainServer::deleteLater);
	connect(pClientSocket, &QTcpSocket::readyRead, this, &MainServer::slotServerRead);
	connect(pClientSocket, &QTcpSocket::disconnected, this, &MainServer::slotClientDisconnected);

	// обработать подключение
//	ui->listWidget->addItem("client connected!");
//	pClientSocket->write("client connected!");
//	sendToClient(pClientSocket, "Server responced: Connected!");
}

void MainServer::slotServerRead()
{
	QTcpSocket* pClientSocket = (QTcpSocket*)sender();
	QDataStream in(pClientSocket);
	in.setVersion(QDataStream::Qt_5_5);

	for (;;) {
		if (!iNextBlockSize) {
			if (pClientSocket->bytesAvailable() < sizeof(quint16)) {
				break;
			}

			in >> iNextBlockSize;
		}

		if (pClientSocket->bytesAvailable() < iNextBlockSize) {
			break;
		}
		QTime time;
		QString str;
		in >> time >> str;

		QString strMessage = time.toString() + " " + "Client has sended: " + str;
//		ui->listWidget->addItem(strMessage);
		iNextBlockSize = 0;

		sendToClient(pClientSocket, "Server received: \"" + str + "\"");
	}
}

void MainServer::slotClientDisconnected()
{
	((QTcpSocket*)sender())->close();
	//	ui->listWidget->addItem("client disconnected!");
}

void MainServer::sendToClient(QTcpSocket *pSocket, const QString &str)
{
	QByteArray arrBlock;
	QDataStream out(&arrBlock, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_5);
	out << quint16(0) << QTime::currentTime() << str;

	out.device()->seek(0);
	out << quint16(arrBlock.size() - sizeof(quint16));

	pSocket->write(arrBlock);
}
