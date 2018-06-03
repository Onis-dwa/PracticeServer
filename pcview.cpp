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
    connect(ui->CloseBtn, SIGNAL(clicked(bool)), SLOT(hideall(bool)));
    connect(ui->RemoteRun, SIGNAL(clicked(bool)), SIGNAL(RemoteRun(bool)));

    cpu = new Graph(86, 193, 261, 20, ui->LDCPU, "CPU@  ", this);
    gpu = new Graph(86, 215, 261, 20, ui->LDGPU, "GPU@  ",this);
    ram = new Graph(86, 237, 261, 20, ui->LDRAM, "RAM@ ",this);
}
pcview::~pcview()
{
    set->close();
	delete set;

    delete cpu;
    delete gpu;
    delete ram;

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
//	ui->LoadCPU->setText(QString::number(dnI.CPU));
//	ui->LoadGPU->setText(QString::number(dnI.GPU));
    //    ui->LoadRAM->setText(QString::number(dnI.RAM));

    ui->LDCPU->setText("CPU:  " + QString::number(dnI.CPU));
    ui->LDGPU->setText("GPU:  " + QString::number(dnI.GPU));
    ui->LDRAM->setText("RAM: " + QString::number(dnI.RAM));
    cpu->AddValue(dnI.CPU);
    gpu->AddValue(dnI.GPU);
    ram->AddValue(dnI.RAM);
}

void pcview::setrand(QString rnd)
{
    if (rnd == "Необходимо подключение")
        ui->lremoterun->setText(rnd);
    else
        ui->lremoterun->setText(" Возвращено " + rnd);
}

void pcview::closeEvent(QCloseEvent *event)
{
    set->hide();
}

void pcview::mouseMoveEvent(QMouseEvent *)
{

}

void pcview::resizeEvent(QResizeEvent *e)
{
    int w = e->size().width() - 95;

    cpu->rs(w, 20);
    gpu->rs(w, 20);
    ram->rs(w, 20);
}

// public slots
void pcview::ShowSettings(bool)
{
	if (!set->isVisible())
        set->show();
}

void pcview::hideall(bool)
{
    set->hide();
    this->hide();
}
