#include "pcset.h"
#include "ui_pcset.h"

// public
PcSet::PcSet(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::PcSet)
{
	ui->setupUi(this);
    this->setWindowTitle("Настройка ПК");

	connect(ui->act, SIGNAL(accepted()), SLOT(accepted()));

	QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
	QRegExp ipRegex ("^" + ipRange
					 + "\\." + ipRange
					 + "\\." + ipRange
					 + "\\." + ipRange + "$");
	QRegExpValidator* ipValidator = new QRegExpValidator(ipRegex, this);
	ui->IP_edit->setValidator(ipValidator);
}
PcSet::~PcSet()
{
	delete ui;
}
void PcSet::setData(const QString &name, const QString &ip)
{
	ui->Name_edit->setText(name);
	ui->IP_edit->setText(ip);
}

// private slots
void PcSet::accepted()
{
	PcSetChanged(ui->Name_edit->text(),
				 ui->IP_edit->text());
}
