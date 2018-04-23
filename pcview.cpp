#include "pcview.h"
#include "ui_pcview.h"

// public
pcview::pcview(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::pcview)
{
	ui->setupUi(this);
	set = new PcSet(this);

	connect(set, SIGNAL(PcSetChanged(QString,QString)), SIGNAL(PcSetChanged(QString,QString)));
	connect(ui->SettingsBtn, SIGNAL(clicked(bool)), SLOT(ShowSettings(bool)));
	connect(ui->CloseBtn, SIGNAL(clicked(bool)), SLOT(hide()));
}
pcview::~pcview()
{
	delete set;
	delete ui;
}
void pcview::setData(const QString &name, const QString &ip)
{
	set->setData(name, ip);
}
void pcview::setData(staticInfo stI)
{
	ui->PCName->setText(stI.PCName);
	ui->OS->setText(stI.OS);
	ui->Bit->setText(stI.Bit);
	ui->CPU->setText(stI.CPU);
	ui->GPU->setText(stI.GPU);
	ui->RAM->setText(stI.RAM);
}
void pcview::setData(dynamicInfo dnI)
{
	ui->LoadCPU->setText(QString::number(dnI.CPU));
	ui->LoadGPU->setText(QString::number(dnI.GPU));
	ui->LoadRAM->setText(QString::number(dnI.RAM));
}

// public slots
void pcview::ShowSettings(bool)
{
	if (!set->isVisible())
		set->show();
}
