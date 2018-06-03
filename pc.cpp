#include "pc.h"
#include "dep.h"

#include "winsock2.h"
#include "iphlpapi.h"
#include "icmpapi.h"

#include <thread>
#include <QMouseEvent>
#include <QApplication>
#include <QMessageBox>

// locale
void ping(pc* ipc)
{
	HANDLE hIcmpFile;						// Обработчик
	unsigned long ipaddr = INADDR_NONE;		// Адрес назначения
	DWORD dwRetVal = 0;						// Количество ответов
	char SendData[8] = "ping";				// Буффер отсылаемых данных
	LPVOID ReplyBuffer = NULL;				// Буффер ответов
	DWORD ReplySize = 0;					// Размер буффера ответов

	// Выделяем память под буффер ответов
	ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
	ReplyBuffer = (VOID*) malloc(ReplySize);

    // Устанавливаем IP-адрес из поля lineEdit
    ipaddr = inet_addr(ipc->GetData()->IP.toStdString().c_str());
    hIcmpFile = IcmpCreateFile();   // Создаём обработчик

    while(!ipc->isConnected) {
		// Вызываем функцию ICMP эхо запроса
		dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData),
					NULL, ReplyBuffer, ReplySize, 2500);

        if (dwRetVal != 0)
            ipc->SetPixmap("PC_CLIENT_DOWN.png");
        else
			ipc->SetPixmap("PC_OFF.png");

        Sleep(1000);
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
	connect(PcView, SIGNAL(PcSetChanged(QString,QString)), SLOT(PcSetChanged(QString,QString)));
    connect(PcView, SIGNAL(RemoteRun(bool)), SLOT(GetRand(bool)));

	NBlockSize = 0;
	Socket = NULL;
	drag = false;
	view = false;
	Dep = NULL;

    isConnected = false;
    // instant ping
    std::thread tping (ping, this);
    tping.detach();

}
pc::~pc() {
	delete img;
	delete name;
	delete layout;

    PcView->close();
	delete PcView;

	if (Socket != NULL)
		delete Socket;
}
void pc::Connect(QTcpSocket* skt) {
	MServ->logfile(name->text() + " connected established from ip: " + ip);


    isConnected = true;
    qApp->processEvents(0, 150);

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

// private slots
void pc::GetRand(bool) {
    if (Socket == NULL)
    {
        PcView->setrand("Необходимо подключение");
        return;
    }

    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_5);

    out << quint8(0) << (QString)"GetRand";

    out.device()->seek(0);
    out << quint8(arrBlock.size() - sizeof(quint8));

    Socket->write(arrBlock);
//    Socket->flush();
}
void pc::ReadClient()
{
	qDebug() << "Read";
	QDataStream in(Socket);
	in.setVersion(QDataStream::Qt_5_5);

	NBlockSize = 0;
	while (true) {
		if (!NBlockSize) {
            if (Socket->bytesAvailable() < sizeof(quint8))
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
        else if (Msg == "GetRand")
        {
            int rnd;
            in >> rnd;
            PcView->setrand(QString::number(rnd));
            NBlockSize = 0;
            break;
        }

		NBlockSize = 0;
	}
}
void pc::Disconnect()
{
	MServ->logfile(name->text() + " disconnect from ip: " + ip);
	Socket->close();

    if (Socket != NULL)
        Socket = NULL;

    isConnected = false;
    // instant ping
    std::thread tping (ping, this);
    tping.detach();

}

void pc::PcSetChanged(QString NAME, QString IP)
{
	name->setText(NAME);
	ip = IP;
	MServ->unsave = true;
	unsaved->show();
}
