#include "pc.h"

#include "winsock2.h"
#include "iphlpapi.h"
#include "icmpapi.h"

//#include <iostream>
//#include <windows.h>
#include <thread>

#include <QTime>

// locale
void ping(pc* ipc)
{
	pc* cpc = ipc;
	int i = 0;

	HANDLE hIcmpFile;						// Обработчик
	unsigned long ipaddr = INADDR_NONE;		// Адрес назначения
	DWORD dwRetVal = 0;						// Количество ответов
	char SendData[8] = "ping";				// Буффер отсылаемых данных
	LPVOID ReplyBuffer = NULL;				// Буффер ответов
	DWORD ReplySize = 0;					// Размер буффера ответов

// setip

	// Выделяем память под буффер ответов
	ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
	ReplyBuffer = (VOID*) malloc(ReplySize);

	// создаём строку, в которою запишем сообщения ответа
	QString strMessage = "";

//if
	// Структура эхо ответа
	PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
	struct in_addr ReplyAddr;
	ReplyAddr.S_un.S_addr = pEchoReply->Address;


	while(true) {
		qDebug() << ipc->GetData()->Name << " - " << i++;
		qDebug() << "ipc = " << ipc;
		qDebug() << "cpc = " << cpc;

		if (ipc != cpc) {
			std::terminate();
		}

		// Устанавливаем IP-адрес из поля lineEdit
		ipaddr = inet_addr(ipc->GetData()->IP.toStdString().c_str());
		hIcmpFile = IcmpCreateFile();   // Создаём обработчик

		//----------------------------------------------------------------------------------------------------
		// Вызываем функцию ICMP эхо запроса
		dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData),
					NULL, ReplyBuffer, ReplySize, 2500);

		if (dwRetVal != 0) {
			if (!ipc->isActive)
				ipc->SetPixmap("PC_CLIENT_DOWN.png");
//			strMessage += "Sent icmp message to " + ipc->GetData()->IP + "\n";
//			if (dwRetVal > 1) {
//				strMessage += "Received " + QString::number(dwRetVal) + " icmp message responses \n";
//				strMessage += "Information from the first response: ";
//			}
//			else {
//				strMessage += "Received " + QString::number(dwRetVal) + " icmp message response \n";
//				strMessage += "Information from the first response: ";
//			}
//			strMessage += "Received from ";
//			strMessage += inet_ntoa( ReplyAddr );
//			strMessage += "\n";
//			strMessage += "Status = " + pEchoReply->Status;
//			strMessage += "Roundtrip time = " + QString::number(pEchoReply->RoundTripTime) + " milliseconds \n";
		}
		else {
			ipc->SetPixmap("PC_OFF.png");
//			strMessage += "Call to IcmpSendEcho failed.\n";
//			strMessage += "IcmpSendEcho returned error: ";
//			strMessage += QString::number(GetLastError());
		}

		qDebug() << strMessage; // Отображаем информацию о полученных данных


		//----------------------------------------------------------------------------------------------------
		Sleep(5000);
	}
	free(ReplyBuffer); // Освобождаем память
}

// public
pc::pc(const pcData& data, QWidget *parent) : QWidget(parent)
{
	img = new QLabel(this);
	img->setPixmap(QPixmap(":/images/pc logo/PC.png"));
	img->setScaledContents(true);
	img->setMinimumHeight(48);
	img->setMaximumHeight(48);
	img->setMinimumWidth(48);
	img->setMaximumWidth(48);

	name = new QLabel(this);
	name->setText(data.Name); // name
	name->setWordWrap(true);
	name->setAlignment(Qt::AlignHCenter);
	name->setAlignment(Qt::AlignTop);
	name->setMinimumHeight(16);
	name->setMaximumHeight(48);
	name->setMinimumWidth(48);
	name->setMaximumWidth(48);

	layout = new QVBoxLayout;
	layout->addWidget(img);
	layout->addWidget(name);
	layout->setMargin(0);
	layout->setSpacing(5);

	this->setLayout(layout);
	this->setGeometry(data.x, data.y, 48, 69); // x,y
	this->show();

	MServ = data.MServ; // MServ
	ip = data.IP; // ip
	set = new PcSet(this);
	isActive = false;
	connect(set, SIGNAL(PcSetChanged(QString,QString)), SLOT(PcSetChanged(QString,QString)));

	// instant ping
	std::thread tping (ping, this);
	tping.detach();

//	std::thread tping (ping);
//	if (tping.joinable())
//		tping.join();

}
pc::~pc() {
	delete img;
	delete name;
	delete layout;
	MServ = NULL;
	Socket = NULL;
}
void pc::Connect(QTcpSocket* skt) {
	MServ->logfile(name->text() + " connected established from ip: " + ip);

	isActive = true;
	Socket = skt;

	connect(Socket, SIGNAL(readyRead()), SLOT(ReadClient()));
	connect(Socket, SIGNAL(disconnected()), SLOT(Disconnect()));

	SetPixmap("PC_ON.png");
}
pcData* pc::GetData()
{
	pcData* dt = new pcData;
	dt->x = this->x();
	dt->y = this->y();
	dt->MServ = MServ;
	dt->Name = name->text();
	dt->IP = ip;

	return dt;
}
void pc::SetPixmap(QString name)
{
	img->setPixmap(QPixmap(":/images/pc logo/" + name));
}

// private
void pc::SendClient(const QString& str) {
	QByteArray arrBlock;
	QDataStream out(&arrBlock, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_5);

	out << quint8(0) << str;
	out.device()->seek(0);
	out << quint8(arrBlock.size() - sizeof(quint8));

	Socket->write(arrBlock);
}

// protected
void pc::mouseMoveEvent(QMouseEvent*)
{
	drag = true;
	int X = this->cursor().pos().x() - MServ->geometry().x();
	int Y = this->cursor().pos().y() - MServ->geometry().y() - 20;

	this->setGeometry(X - lx, Y - ly, 48, 101);
	MServ->unsave = true;
}
void pc::mousePressEvent(QMouseEvent *)
{
	drag = false;
	lx = this->cursor().pos().x() - MServ->geometry().x() - this->geometry().x();
	ly = this->cursor().pos().y() - MServ->geometry().y() - 20 - this->geometry().y();
}
void pc::mouseReleaseEvent(QMouseEvent *)
{
	if (!drag) {
		if (!set->isVisible()){
			set->setData(name->text(), ip);

			set->show();
		}
	}
}

// private slots
void pc::ReadClient()
{
	QDataStream in(Socket);
	in.setVersion(QDataStream::Qt_5_5);

	while (true) {
		if (!NBlockSize) {
			if (Socket->bytesAvailable() < sizeof(quint16))
				break;

			in >> NBlockSize;
		}

		if (Socket->bytesAvailable() < NBlockSize)
			break;

//		bool fuck;

//		QTime time;
//		time.
		NBlockSize = 0;

		SendClient("accepted");
	}
}
void pc::Disconnect()
{
	MServ->logfile(name->text() + " disconnect from ip: " + ip);
	isActive = false;

	Socket->close();
}

void pc::PcSetChanged(QString NAME, QString IP)
{
	name->setText(NAME);
	ip = IP;
	MServ->unsave = true;
}
