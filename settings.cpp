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
					   const QString &namePcsFile)
{
	Ip = ip;
	Port = port;
	Nlf = nameLogFile;
	Npf = namePcsFile;

	ui->IP->setText(ip);
	ui->Port->setText(QString::number(port));
	ui->LogfileName->setText(nameLogFile);
	ui->PCfileName->setText(namePcsFile);
}


// private slots
void Settings::accepted()
{
	settingsChanged(ui->Port->text().toShort(),
					ui->LogfileName->text(),
					ui->PCfileName->text());
}
