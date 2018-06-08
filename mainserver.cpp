#include "mainserver.h"
#include "ui_mainserver.h"
#include "ws.h"
#include "pc.h"
#include "dep.h"
#include "settings.h"
#include "pcset.h"

#include <QMessageBox>
#include <QDateTime>
#include <QCloseEvent>
#include <QNetworkInterface>

// public
MainServer::MainServer(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainServer)
{
    this->setWindowTitle("Сервер");
	// SetUP
	loadSettings();

	mTcpServer = new QTcpServer(this);
    if (!mTcpServer->listen(QHostAddress::AnyIPv4, Serverport)){
		QMessageBox::critical(0,
							  "Server error",
							  "Unable to srart the server:" + mTcpServer->errorString());

        mTcpServer->close();
        this->close();
		logfile("Server is not started");
		return;
	}
	logfile("Server started");

	// UploadUI
	ui->setupUi(this);
	ui->HomeButton->hide();
	winset = new Settings(this);
	ws = new WS(this, ui->MainWidget);
	ws->setGeometry(0, 0, 25000, 25000);
	ui->Center->stackUnder(ui->Menu);
	ui->Center->setGeometry((this->width() - 4) / 2,
							(this->height() - 4) / 2,
							4, 4);
    ui->Center->hide();
	ws->stackUnder(ui->Center);
	this->showMaximized();

	// null's
	mTcpSocket = NULL;
	iNextBlockSize = 0;
	unsave = false;
	pclist = new QList<pc*>();
	deplist = new QList<dep*>();

	// connect's
	connect(mTcpServer, SIGNAL(newConnection()), SLOT(slotNewConnection()));
	connect(winset, SIGNAL(settingsChanged(quint16, QString, QString)),
					SLOT(settingsChanged(quint16, QString, QString)));
	connect(ui->SaveButton, SIGNAL(clicked(bool)), SLOT(saveFile(bool)));
	connect(ui->SettingsButon, SIGNAL(clicked(bool)), SLOT(menuSettings(bool)));
	connect(ui->AddPC, SIGNAL(toggled(bool)), SLOT(AddPc(bool)));
	connect(ui->AddDep, SIGNAL(toggled(bool)), SLOT(AddDep(bool)));
    connect(ui->RemovePC, SIGNAL(toggled(bool)), SLOT(RemovePC(bool)));
	connect(ui->HomeButton, SIGNAL(clicked(bool)), SLOT(GoHome(bool)));

	// load
	loadData();
}
MainServer::~MainServer()
{
	logfile("Server closed");
    delete ui;
}
void MainServer::logfile(const QString& str)
{
    if (!flog->exists())
        qDebug() << "log file does not exist";
    if (!flog->open(QIODevice::Append | QIODevice::Text)) {
        qDebug() << "Failed write to log file";

        if (!flog->open(QIODevice::WriteOnly | QIODevice::Text))
            return;
    }

	QTextStream out(flog);
	out << QDateTime::currentDateTime().date().year() << "." <<
		   QDateTime::currentDateTime().date().month() << "." <<
		   QDateTime::currentDateTime().date().day() << "-" <<
		   QDateTime::currentDateTime().time().hour() << ":" <<
		   QDateTime::currentDateTime().time().minute() << ":" <<
           QDateTime::currentDateTime().time().second() << "	" <<
		   str << "\n";
	flog->close();
}
void MainServer::moveWS(int x, int y)
{
	ws->move(ws->x() - x, ws->y() - y);

	ui->Center->move(ui->Center->x() - x, ui->Center->y() - y);

	if (ui->Center->x() < 0 ||
		ui->Center->x() > this->width() ||
		ui->Center->y() < 51 ||
		ui->Center->y() > this->height())
	{
		ui->HomeButton->show();
	}
	else
		ui->HomeButton->hide();
}
int MainServer::tryMove(pc *P, int &newx, int &newy)
{
	if (P->Dep != NULL)
	{
		if (!(P->x() >= P->Dep->x() &&
			  P->x() <= P->Dep->x() + P->Dep->width() &&
			  P->y() >= P->Dep->y() &&
			  P->y() <= P->Dep->y() + P->Dep->height()))
		{
			P->stackUnder(P->Dep);
			P->Dep = NULL;
		}
	}

	bool x = true;
	bool y = true;

	pc* p;
	dep* d;

    int w = P->width();
    int h = P->height();

	// pc's
	for(int i = 0; i < pclist->count(); i++) {
		p = pclist->at(i);
		if (p != P)
			if (// Left Top
				(newx		>= p->x() &&
                 newx		<= p->x() + w &&
				 newy		>= p->y() &&
                 newy		<= p->y() + h) ||
				// Left Bottom
				(newx		>= p->x() &&
                 newx		<= p->x() + w &&
                 newy + h	>= p->y() &&
                 newy + h	<= p->y() + h) ||
				// Right Top
                (newx + w	>= p->x() &&
                 newx + w	<= p->x() + w &&
				 newy		>= p->y() &&
                 newy		<= p->y() + h) ||
				// Right Bottom
                (newx + w	>= p->x() &&
                 newx + w	<= p->x() + w &&
                 newy + h	>= p->y() &&
                 newy + h	<= p->y() + h))
			{
                if (P->y()		< p->y() + w &&
                    P->y() + w > p->y()) { // Left & Right
					x = false;
				}
				else { // Top & Bot
					y = false;
				}
			}
	}

	// dep's
	for(int i = 0; i < deplist->count(); i++) {
		d = deplist->at(i);
		if (// Left Top
				(newx		>= d->x() &&
				 newx		<= d->x() + d->width() &&
				 newy		>= d->y() &&
				 newy		<= d->y() + d->height()) ||
				// Left Bottom
				(newx		>= d->x() &&
				 newx		<= d->x() + d->width() &&
                 newy + h	>= d->y() &&
                 newy + h	<= d->y() + d->height()) ||
				// Right Top
                (newx + w	>= d->x() &&
                 newx + w	<= d->x() + d->width() &&
				 newy		>= d->y() &&
				 newy		<= d->y() + d->height()) ||
				// Right Bottom
                (newx + w	>= d->x() &&
                 newx + w	<= d->x() + d->width() &&
                 newy + h	>= d->y() &&
                 newy + h	<= d->y() + d->height()))
		{
			if (newx		> d->x() &&
				newy		> d->y() &&
                newx + w	< d->x() + d->width() &&
                newy + h	< d->y() + d->height())
			{
				d->DropDown(true);

				P->raise();
				P->Dep = d;
			}
			else if (P->y()		< d->y() + d->height() &&
                P->y() + h	> d->y()) { // Left & Right
				x = false;
			}
			else { // Top & Bot
				y = false;
			}
		}
		else
		{
			d->DropDown(false);
		}
	}

	if (x && y)
		return 1;
	else if (x && !y)
		return 2;
	else if (!x && y)
		return 3;
	else
		return 0;
}
int MainServer::tryDepMove(dep *P, int &newx, int &newy)
{
	bool x = true;
	bool y = true;
	dep* d;

	for(int i = 0; i < deplist->count(); i++) {
		d = deplist->at(i);
		if (d != P)
		{
			if (// Left Top
					(newx				>= d->x() &&
					 newx				<= d->x() + d->width() &&
					 newy				>= d->y() &&
					 newy				<= d->y() + d->height()) ||
					// Left Bottom
					(newx				>= d->x() &&
					 newx				<= d->x() + d->width() &&
					 newy + P->height()	>= d->y() &&
					 newy + P->height()	<= d->y() + d->height()) ||
					// Right Top
					(newx + P->width()	>= d->x() &&
					 newx + P->width()	<= d->x() + d->width() &&
					 newy				>= d->y() &&
					 newy				<= d->y() + d->height()) ||
					// Right Bottom
					(newx + P->width()	>= d->x() &&
					 newx + P->width()	<= d->x() + d->width() &&
					 newy + P->height()	>= d->y() &&
					 newy + P->height()	<= d->y() + d->height()))
			{
				if (P->y()				< d->y() + d->height() &&
					P->y() + P->height()> d->y()) { // Left & Right
					x = false;
				}
				else { // Top & Bot
					y = false;
				}
			}
		}
	}

	if (x && y)
		return 1;
	else if (x && !y)
		return 2;
	else if (!x && y)
		return 3;
	else
		return 0;
}
WS *MainServer::GetWS()
{
    return ws;
}
bool MainServer::ExistName(QString &name)
{
    for (int i = 0; i < pclist->count(); i++)
    {
        if (pclist->at(i)->GetData()->Name == name)
            return true;
    }
    return false;
}
bool MainServer::ExistIP(QString &ip)
{
    for (int i = 0; i < pclist->count(); i++)
    {
        if (pclist->at(i)->GetData()->IP == ip)
            return true;
    }
    return false;
}

// private
void MainServer::loadSettings()
{
	QString strlog;
	QString strdata;

	fsettings = new QFile("settings.txt");
	if (fsettings->exists()) {
		if (fsettings->open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream in (fsettings);

			in >> Serverport >> strlog >> strdata;
		}
	}
	else {
		if (fsettings->open(QIODevice::WriteOnly | QIODevice::Text)) {
			QTextStream out (fsettings);

			Serverport = 32094;
			strlog = "log.txt";
			strdata= "data.txt";

			out << Serverport << "\n" << strlog << "\n" << strdata;
		}
	}

	flog = new QFile(strlog);
	fdata= new QFile(strdata);
	fsettings->close();
}
void MainServer::loadData()
{
	if (!fdata->exists()) {
		qDebug() << "Data file error. File does not exists";
		return;
	}
	if (!fdata->open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Data file error. Failed read to file";
		return;
	}

	QString name;
	pcData pdt;
	depData ddt;
	QTextStream in (fdata);
	while (!in.atEnd()) {
		in >> name;
		if (name == "1")
		{
			in >> ddt;
			deplist->push_back(new dep(ddt, (QFrame*)ws, this));
			deplist->last()->unsaved->hide();
		}
		else if (name == "2")
		{
			in >> pdt;

            if (!ExistIP(pdt.IP) && !ExistName(pdt.Name))
            {
                pclist->push_back(new pc(pdt, deplist->last(), this));
                pclist->last()->unsaved->hide();
                pclist->last()->Dep = deplist->last();
            }
		}
		else if (name == "3")
		{
			in >> pdt;

            if (!ExistIP(pdt.IP) && !ExistName(pdt.Name))
            {
                pclist->push_back(new pc(pdt, ws, this));
                pclist->last()->unsaved->hide();
            }
		}
	}
	fdata->close();

    for (int i = 0; i < pclist->count(); i++)
        pclist->at(i)->RunPing();
}
void MainServer::resizeEvent(QResizeEvent*)
{
    ui->Menu->setGeometry(0, 0, this->width(), 61);
	ui->Center->move((this->width() - 4) / 2,
					 (this->height() - 4) / 2);
	ws->move(ui->Center->x() - 12500,
			 ui->Center->y() - 12500);
}

// protected
void MainServer::closeEvent(QCloseEvent *event)
{
	if (!unsave) {
		event->accept();
		return;
	}

	int choose = QMessageBox::warning(this, "Изменения не сохранены!", "Сохранить изменения?", "Сохранить", "Не сохранять", "Отмена", 0, 1);

	if (choose == 0) {
		saveFile(false);
		event->accept();
	}
	else if (choose == 1) {
		event->accept();
	}
	else {
		event->ignore();
	}
}

// public slot
void MainServer::slotNewConnection()
{
	mTcpSocket = mTcpServer->nextPendingConnection();

	if (pclist->count() < 1) {
		mTcpSocket->close();
		return;
	}

//	qDebug() << "New connection";
	for (int i = 0; i < pclist->count(); i++) {
//		qDebug() << "client" << mTcpSocket->peerAddress();
//		qDebug() << "ip: " + pclist->at(i)->GetData()->IP;
		if (mTcpSocket->peerAddress().toString() == pclist->at(i)->GetData()->IP) {
			pclist->at(i)->Connect(mTcpSocket);

			break;
		}
	}
}
void MainServer::saveFile(bool)
{
	if (fdata->open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream out (fdata);

		for (int j = 0; j < pclist->count(); j++)
			pclist->at(j)->unsaved->show();

		for (int i = 0; i < deplist->count(); i++) {
			out << "1\n"
				<< *(deplist->at(i)->GetData()) << "\n";
			deplist->at(i)->unsaved->hide();

			for (int j = 0; j < pclist->count(); j++) {
				if (pclist->at(j)->Dep == deplist->at(i) && pclist->at(j)->unsaved->isVisible())
				{
					out << "2\n" << *(pclist->at(j)->GetData()) << "\n";
					pclist->at(j)->unsaved->hide();
                    pclist->at(j)->Rename();
				}
			}
		}
		for (int j = 0; j < pclist->count(); j++) {
			if (pclist->at(j)->unsaved->isVisible())
			{
                out << "3\n" << *(pclist->at(j)->GetData()) << "\n";
                pclist->at(j)->unsaved->hide();
                pclist->at(j)->Rename();
			}
		}

		fdata->close();

		logfile("data file update");
		unsave = false;
	}
}
void MainServer::menuSettings(bool)
{
	if (!winset->isVisible())
	{
		QString addr;
		foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
			if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
				 addr = address.toString();
		}

		winset->setData(addr,
						mTcpServer->serverPort(),
						flog->fileName(),
						fdata->fileName());
		winset->show();
	}
}
void MainServer::settingsChanged(quint16 port, QString nlf, QString ndf)
{
	mTcpServer->close();
	mTcpServer = new QTcpServer(this);
	if (!mTcpServer->listen(QHostAddress::AnyIPv4, port)){
		QMessageBox::critical(0, "Server error", "Unable to change the port:" + mTcpServer->errorString());

        mTcpServer->close();
		this->close();
		return;
	}

	flog->setFileName(nlf);
	fdata->setFileName(ndf);

	if (fsettings->open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream out (fsettings);
		out << port << "\n" << nlf << "\n" << ndf;
		fsettings->close();
	}
	logfile("settings changed to: " + QString::number(port) + " " + nlf + " " + ndf);
}
void MainServer::newPC(int X, int Y)
{
    pcs = new PcSet(this);
    pcs->setWindowTitle("Ввод данных пк");
    pcs->show();

    pcData dt;
    dt.x = X;
    dt.y = Y;
    dt.Name = "";
    dt.IP = "";

    pclist->push_back(new pc(dt, ws, this));
    pclist->last()->lower();
    AddPc(false);
    unsave = true;

    connect(pcs, SIGNAL(PcSetChanged(QString,QString)), pclist->last(), SLOT(PcDataSet(QString,QString)));
    connect(pcs, SIGNAL(rejected()), this, SLOT(rejectPC()));
}
void MainServer::newDep(int x, int y)
{
	// заставить ввести данные
	depData dt;
	dt.x = x;
	dt.y = y;
	dt.Name = "New dep";
	dt.width = 350;
	dt.heigth = 350;

	deplist->push_back(new dep(dt, (QFrame*)ws, this));
	deplist->last()->raise();
	AddDep(false);
    unsave = true;
}
void MainServer::acceptPC()
{
    pcs->close();
    delete pcs;
}
void MainServer::rejectPC()
{
    pcs->close();
    delete pcs;

    pclist->last()->~pc();
    pclist->pop_back();
}
void MainServer::RemovePC(pc* p)
{
    for (int i = 0; i < pclist->count(); i++)
    {
        if (p->GetData()->IP == pclist->at(i)->GetData()->IP)
        {
            p->~pc();
            pclist->removeAt(i);
        }
    }
    unsave = true;
}
void MainServer::AddPc(bool state)
{
	if (state)
    {
        ui->RemovePC->setEnabled(false);
        RemovePC(false);
        ui->AddDep->setEnabled(false);
        ws->toggleAddDep(false);


        ui->AddPC->setChecked(true);
        ws->toggleAddPC(true);
	}
	else
	{
        ui->AddDep->setEnabled(true);
        ui->RemovePC->setEnabled(true);


        ui->AddPC->setChecked(false);
        ws->toggleAddPC(false);
	}
}
void MainServer::AddDep(bool state)
{
	if (state)
    {
        ui->RemovePC->setEnabled(false);
        RemovePC(false);
        ui->AddPC->setEnabled(false);
        ws->toggleAddPC(false);


        ui->AddDep->setChecked(true);
        ws->toggleAddDep(true);
	}
	else
    {
        ui->AddPC->setEnabled(true);
        ui->RemovePC->setEnabled(true);


        ui->AddDep->setChecked(false);
        ws->toggleAddDep(false);
    }
}
void MainServer::RemovePC(bool state)
{
    if (state)
    {
        ui->AddPC->setEnabled(false);
        ws->toggleAddPC(false);
        ui->AddDep->setEnabled(false);
        ws->toggleAddDep(false);

        for (int i = 0; i < pclist->count(); i++)
            pclist->at(i)->Remove(true);

        ui->RemovePC->setChecked(true);
    }
    else
    {
        ui->AddPC->setEnabled(true);
        ui->AddDep->setEnabled(true);

        for (int i = 0; i < pclist->count(); i++)
            pclist->at(i)->Remove(false);

        ui->RemovePC->setChecked(false);
    }
}
void MainServer::GoHome(bool)
{
	ui->Center->move((this->width() - 4) / 2,
					 (this->height() - 4) / 2);
	ws->move(ui->Center->x() - 12500,
			 ui->Center->y() - 12500);

	ui->HomeButton->hide();
}
