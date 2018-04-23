#include "pc.h"
#include "dep.h"

#include "winsock2.h"
#include "iphlpapi.h"
#include "icmpapi.h"

#include <thread>
#include <QTime>
#include <QMouseEvent>

// locale
void ping(pc* ipc)
{
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
		//qDebug() << ipc->GetData()->Name << " - " << i++;
//		qDebug() << "ipc = " << ipc;
//		//qDebug() << "cpc = " << cpc;
		//	ЛОЛ ПОТОК МОЖНО НЕ ЗАВЕРШАТЬ!?!?!?!?
//		if (ipc == NULL) {
//			qDebug() << "ipc = " << ipc;
//			std::terminate();
//			break;
//		}

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

		//qDebug() << strMessage; // Отображаем информацию о полученных данных


		//----------------------------------------------------------------------------------------------------
		Sleep(5000);
	}
	free(ReplyBuffer); // Освобождаем память
}

// public
pc::pc(const pcData& data, QWidget *parent, MainServer* MServer) : QWidget(parent)
{
	unsaved = new QLabel(this);
	unsaved->setPixmap(QPixmap(":/res/resources/Unsaved.png"));
	unsaved->setScaledContents(true);

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

	QPalette palette;
	palette.setColor(QPalette::Window, Qt::white);
	palette.setColor(QPalette::WindowText, Qt::white);
	name->setPalette(palette);

	layout = new QVBoxLayout;
	layout->addWidget(img);
	layout->addWidget(name);
	layout->setMargin(0);
	layout->setSpacing(5);

	this->setLayout(layout);
	this->setGeometry(data.x, data.y, 48, 69); // x,y
	unsaved->raise();
	unsaved->setGeometry(0, 30, 16, 16);
	this->show();

	MServ = MServer;
	ip = data.IP; // ip
	PcView = new pcview(this);
	isActive = false;
	connect(PcView, SIGNAL(PcSetChanged(QString,QString)), SLOT(PcSetChanged(QString,QString)));

	// instant ping
	std::thread tping (ping, this);
	tping.detach();

	NBlockSize = 0;
	Socket = NULL;
	drag = false;
	view = false;
	Dep = NULL;
}
pc::~pc() {
	delete img;
	delete name;
	delete layout;
	delete PcView;

	if (Socket != NULL)
		delete Socket;
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
void pc::mousePressEvent(QMouseEvent* m)
{
	if (m->button() & Qt::RightButton)
	{
		if (this->parent() == Dep)
		{
			this->setParent((QWidget*)MServ->GetWS());
			this->move(this->x() + Dep->x(), this->y() + Dep->y());
			this->show();
		}

		drag = true;
		DragX = this->cursor().pos().x();
		DragY = this->cursor().pos().y();
	}
	else
		view = true;
}
void pc::mouseMoveEvent(QMouseEvent*)
{
	if (drag)
	{
		int TX = this->x() - (DragX - this->cursor().pos().x());
		int TY = this->y() - (DragY - this->cursor().pos().y());
		int res = MServ->tryMove(this, TX, TY);

		switch (res) {
		case 1:
			this->move(TX, TY);
			DragX = this->cursor().pos().x();
			DragY = this->cursor().pos().y();
			break;
		case 2:
			this->move(TX, this->y());
			DragX = this->cursor().pos().x();
			break;
		case 3:
			this->move(this->x(), TY);
			DragY = this->cursor().pos().y();
			break;
		}

		MServ->unsave = true;
		unsaved->show();
	}
	else
		view = false;
}
void pc::mouseReleaseEvent(QMouseEvent*)
{
	if (view) {
		if (!PcView->isVisible()){
			PcView->setData(name->text(), ip);

			PcView->show();
		}
		view = false;
	}
	else
	{
		drag = false;

		if (Dep != NULL && this->parent() != Dep)
		{
			this->setParent(Dep);
			this->move(this->x() - Dep->x(), this->y() - Dep->y());
			this->show();
		}
	}
}

// public slots
void pc::ReadClient()
{
	qDebug() << "Read";
	QDataStream in(Socket);
	in.setVersion(QDataStream::Qt_5_5);

	NBlockSize = 0;
	while (true) {
		if (!NBlockSize) {
			if (Socket->bytesAvailable() < sizeof(quint16))
				break;

			in >> NBlockSize;
		}

		if (Socket->bytesAvailable() < NBlockSize)
			break;

		QString Msg;
		in >> Msg;
		qDebug() << Msg;

		if (Msg == "Static")
		{
			staticInfo stI;
			in >> stI;

			qDebug() << stI.PCName;
			qDebug() << stI.OS;
			qDebug() << stI.Bit;
			qDebug() << stI.CPU;
			qDebug() << stI.GPU;
			qDebug() << stI.RAM;
			PcView->setData(stI);
			NBlockSize = 0;
			break;
		}
		else if (Msg == "Dynamic")
		{
			dynamicInfo dnI;
			in >> dnI;

			qDebug() << dnI.CPU;
			qDebug() << dnI.GPU;
			qDebug() << dnI.RAM;
			PcView->setData(dnI);
			NBlockSize = 0;
			break;
		}

		NBlockSize = 0;
	}
}


// private slots
#include <QMessageBox>
struct strct
{
	quint8 a;
	QString b;
	bool c;
	float d;

	QDataStream& operator >>(QDataStream& in)
	{
		return in << a << b << c << d;
	}
	QDataStream& operator <<(QDataStream& in)
	{
		return in >> a >> b >> c >> d;
	}
};
struct strct2
{
	QString a;
	QString b;
	QString c;
	quint8 d;
	bool e;
	float f;
	QString g;

	friend QDataStream& operator >>(QDataStream& in, strct2& st)
	{
		return in >> st.a >> st.b >> st.c >> st.d >> st.e >> st.f >> st.g;
	}
	friend QDataStream& operator <<(QDataStream& in, strct2& st)
	{
		return in << st.a << st.b << st.c << st.d << st.e << st.f << st.g;
	}
};
void pc::ReadClient2()
{
	QDataStream in(Socket);
	in.setVersion(QDataStream::Qt_5_5);

	NBlockSize = 0;
	while (true) {
		if (!NBlockSize) {
			if (Socket->bytesAvailable() < sizeof(quint16))
				break;

			in >> NBlockSize;
		}

		if (Socket->bytesAvailable() < NBlockSize)
			break;

		QString Msg;
		in >> Msg;

		if (Msg == "-t")
		{
			strct st;
			//st.operator >>(in);
			//in >> st;
			st << in;

			qDebug() << st.a;
			qDebug() << st.b;
			qDebug() << st.c;
			qDebug() << st.d;
		}
		else if (Msg == "-w")
		{
			strct2 st;
			in >> st;

			qDebug() << st.a;
			qDebug() << st.b;
			qDebug() << st.c;
			qDebug() << st.d;
			qDebug() << st.e;
			qDebug() << st.f;
			qDebug() << st.g;
		}

		NBlockSize = 0;

		//SendClient("accepted");
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
	unsaved->show();
}
