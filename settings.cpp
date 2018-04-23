#include "settings.h"
#include "ui_settings.h"

// public
Settings::Settings(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Settings)
{
	ui->setupUi(this);

	connect(ui->act, SIGNAL(accepted()), SLOT(accepted()));
}
Settings::~Settings()
{
	delete ui;
}
void Settings::setData(const QString &ip,
					   const quint16 &port,
					   const QString &nameLogFile,
					   const QString &nameDataFile)
{
	Ip = ip;
	Port = port;
	Nlf = nameLogFile;
	Ndf = nameDataFile;

	ui->IP->setText(ip);
	ui->Port->setText(QString::number(port));
	ui->LogfileName->setText(nameLogFile);
	ui->DatafileName->setText(nameDataFile);
}


// private slots
void Settings::accepted()
{
	settingsChanged(ui->Port->text().toShort(),
					ui->LogfileName->text(),
					ui->DatafileName->text());
}
