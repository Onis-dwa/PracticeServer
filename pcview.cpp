#include "pcview.h"
#include "ui_pcview.h"

// public
pcview::pcview(const QString& name, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::pcview)
{
	ui->setupUi(this);
    this->setWindowTitle(name);
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
    delete ui;
}
void pcview::setData(const QString &name, const QString &ip)
{
	set->setData(name, ip);
}
void pcview::setData(staticInfo stI)
{
    if ((ui->PCName->text() != "unnown" && ui->PCName->text() != stI.PCName) ||
        (ui->OS->text() != "unnown" && ui->OS->text() != stI.OS) ||
        (ui->Bit->text() != "unnown" && ui->Bit->text() != stI.Bit) ||
        (ui->CPU->text() != "unnown" && ui->CPU->text() != stI.CPU) ||
        (ui->GPU->text() != "unnown" && ui->GPU->text() != stI.GPU) ||
        (ui->RAM->text() != "unnown" && ui->RAM->text() != stI.RAM))
        emit HeadChanged();


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
QString pcview::GetStatic()
{
    return "	" +
            ui->PCName->text() + "\n	" +
            ui->OS->text() + "\n	" +
            ui->Bit->text() + "\n	" +
            ui->CPU->text() + "\n	" +
            ui->GPU->text() + "\n	" +
            ui->RAM->text() + "\n";
}
QString pcview::GetDynaminc()
{
    return "	CPU:	Min: " +
                             QString::number(cpu->min) +
           "%\n		Max: " + QString::number(cpu->max) +
           "%\n		Avg: " + QString::number(cpu->avg) +

           "%\n	GPU:	Min: " +
                             QString::number(gpu->min) +
           "%\n		Max: " + QString::number(gpu->max) +
           "%\n		Avg: " + QString::number(gpu->avg) +

           "%\n	RAM:	Min: " +
                             QString::number(ram->min) +
           "%\n		Max: " + QString::number(ram->max) +
           "%\n		Avg: " + QString::number(ram->avg) + "%";
}

// protected
void pcview::closeEvent(QCloseEvent *)
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
