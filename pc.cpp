#include "pc.h"
#include "dep.h"

#include "winsock2.h"
#include "iphlpapi.h"
#include "icmpapi.h"

#include <thread>
#include <QMouseEvent>
#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QDir>

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

    QString ip = ipc->GetData()->IP;

    while(!ipc->isConnected || ip != ipc->GetData()->IP) {
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
    img->setMinimumHeight(64);
    img->setMaximumHeight(64);
    img->setMinimumWidth(64);
    img->setMaximumWidth(64);

	name = new QLabel(this);
	name->setText(data.Name); // name
	name->setWordWrap(true);
    name->setFont(QFont("MS Shell Dlg 2", 11));
    name->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    name->setMinimumHeight(16);
    name->setMaximumHeight(48);
    name->setMinimumWidth(64);
    name->setMaximumWidth(64);

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
    this->setGeometry(data.x, data.y, 64, 85); // x,y
	unsaved->raise();
    unsaved->setGeometry(10, 10, 16, 16);
	this->show();

	MServ = MServer;
	ip = data.IP; // ip
    PcView = new pcview(data.Name, this);
	connect(PcView, SIGNAL(PcSetChanged(QString,QString)), SLOT(PcSetChanged(QString,QString)));
    connect(PcView, SIGNAL(RemoteRun(bool)), SLOT(GetRand(bool)));
    connect(PcView, SIGNAL(HeadChanged()), SLOT(HeadChanged()));

	NBlockSize = 0;
	Socket = NULL;
	drag = false;
	view = false;
	Dep = NULL;
    remove = false;
    headerchanged = false;
    unsaved_data = false;
    bload = false;
    rename = false;
}
pc::~pc() {
    Save();
    return;

    qDebug() << "pc delete unsaved";
    delete unsaved;
    qDebug() << "pc delete img";
	delete img;
    qDebug() << "pc delete name";
	delete name;
    qDebug() << "pc delete layout";
	delete layout;

    qDebug() << "pc delete PcView";
    PcView->close();
	delete PcView;

    Dep = NULL;
    MServ = NULL;
	if (Socket != NULL)
    {
        qDebug() << "pc close Socket";
        Socket->disconnectFromHost();
        qDebug() << "pc delete Socket";
        Socket = NULL;
    }
}
void pc::Connect(QTcpSocket* skt) {
	MServ->logfile(name->text() + " connected established from ip: " + ip);


    isConnected = true;
//    qApp->processEvents(0, 150);

	Socket = skt;

	connect(Socket, SIGNAL(readyRead()), SLOT(ReadClient()));
	connect(Socket, SIGNAL(disconnected()), SLOT(Disconnect()));

	SetPixmap("PC_ON.png");

    timer.start();
    unsaved_data = true;
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
void pc::Remove(bool state)
{
    if (state)
    {
        remove = true;
        this->setCursor(QCursor(Qt::CrossCursor));
    }
    else
    {
        remove = false;
        this->setCursor(QCursor(Qt::ArrowCursor));
    }
}
void pc::RunPing()
{
    Load();

    isConnected = false;
    // instant ping
    std::thread tping (ping, this);
    tping.detach();
}
void pc::Rename()
{
    if (rename)
    {
        QFile* fdata;
        fdata = new QFile(QApplication::applicationDirPath() + "/PCData/" + oldname + ".txt");

        fdata->rename(name->text());
    }
}

// private
void pc::Load()
{
    if (bload)
        return;
    QFile* fdata;
    fdata = new QFile(QApplication::applicationDirPath() + "/PCData/" + name->text() + ".txt");
    if (!fdata->exists()) {
        qDebug() << "Data file error. " + name->text() + " file does not exists";
        return;
    }
    if (!fdata->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Data file error. " + name->text() + " failed read to file";
        return;
    }

    quint64 pos = 0;
    QString str;

    QTextStream in(fdata);
    while(!in.atEnd())
    {
        str = in.readLine();
        if (str == "PC config changed!" || str == "PC config:")
        {
            pos = in.pos();
        }
    }

    staticInfo st;
    if (pos != 0)
    {
        in.seek(pos);

        in >> st;
        PcView->setData(st);
    }
    bload = true;
}
void pc::Save()
{
    if (!unsaved_data)
        return;

    QDir dr(QApplication::applicationDirPath());
    if (!dr.exists("PCData"))
        dr.mkdir("PCData");

    QFile* fdata;
    fdata = new QFile(QApplication::applicationDirPath() + "/PCData/" + name->text() + ".txt");

    if (fdata->exists()) {
        if (fdata->open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(fdata);
            // current time
            out << QDateTime::currentDateTime().date().year() << "." <<
                   QDateTime::currentDateTime().date().month() << "." <<
                   QDateTime::currentDateTime().date().day() << "-" <<
                   QDateTime::currentDateTime().time().hour() << ":" <<
                   QDateTime::currentDateTime().time().minute() << ":" <<
                   QDateTime::currentDateTime().time().second() << "\n";
            if (headerchanged)
                out << "PC config changed!\n" + PcView->GetStatic();

            out << "Working time: " + ParseTime() + "\n";
            out << "During this time: \n" + PcView->GetDynaminc() << "\n\n\n";
        }
    }
    else {
        if (fdata->open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(fdata);
            // current time
            out << QDateTime::currentDateTime().date().year() << "." <<
                   QDateTime::currentDateTime().date().month() << "." <<
                   QDateTime::currentDateTime().date().day() << "-" <<
                   QDateTime::currentDateTime().time().hour() << ":" <<
                   QDateTime::currentDateTime().time().minute() << ":" <<
                   QDateTime::currentDateTime().time().second() << "\n";

            out << "PC details:\n	Name:	" + name->text() + "\n	IP:	" + ip + "\n";
            out << "PC config:\n" + PcView->GetStatic();
            out << "Working time: " + ParseTime() + "\n";
            out << "During this time: \n" + PcView->GetDynaminc() << "\n\n\n";
        }
    }

    unsaved_data = false;
    fdata->close();
}
QString pc::ParseTime()
{
    int i = timer.elapsed();

    int s,m,h,d;

    d = i / 86400000;
    if (d > 0)
        i -= d * 86400000;

    h = i / 3600000;
    if (h > 0)
        i -= h * 3600000;

    m = i / 60000;
    if (m > 0)
        i -= m * 60000;

    s = i / 1000;
    if (s > 0)
        i -= s * 1000;

    QString str = "";
    if (d > 0)
        str += QString::number(d) + " days ";
    if (h > 0)
        str += QString::number(h) + " hours ";
    if (m > 0)
        str += QString::number(m) + " minutes ";
    if (s > 0)
        str += QString::number(s) + " seconds ";
    if (i > 0)
        str += QString::number(i) + " milliseconds ";

    return str;
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
    if (remove)
    {
        MServ->RemovePC(this);
    }
    else if (view) {
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

		if (Msg == "Static")
		{
			staticInfo stI;
            in >> stI;

			PcView->setData(stI);
			NBlockSize = 0;
			break;
		}
		else if (Msg == "Dynamic")
		{
			dynamicInfo dnI;
			in >> dnI;

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

    if (Socket != NULL)
    {
        Socket->disconnectFromHost();
        Socket = NULL;
    }
    Save();

    RunPing();
}
void pc::PcDataSet(QString NAME, QString IP)
{
    if (MServ->ExistIP(IP))
    {
        QMessageBox::critical(0,
                      "Create pc error",
                      "this ip already exists:");

        MServ->rejectPC();
        return;
    }
    if (MServ->ExistName(NAME))
    {
        QMessageBox::critical(0,
                      "Create pc error",
                      "this name already exists:");

        MServ->rejectPC();
        return;
    }
    MServ->acceptPC();

    name->setText(NAME);
    ip = IP;
    MServ->unsave = true;
    unsaved->show();

    RunPing();
}
void pc::PcSetChanged(QString NAME, QString IP)
{
    if (MServ->ExistIP(IP))
    {
        QMessageBox::critical(0,
                      "Create pc error",
                      "this ip already exists:");
        return;
    }
    if (MServ->ExistName(NAME))
    {
        QMessageBox::critical(0,
                      "Create pc error",
                      "this name already exists:");
        return;
    }

    rename = true;
    oldname = name->text();
	name->setText(NAME);
	ip = IP;
	MServ->unsave = true;
	unsaved->show();
    if (Socket != NULL)
        Socket->close();
    qApp->processEvents(0, 150);

    RunPing();

}
void pc::HeadChanged()
{
    headerchanged = true;
}
