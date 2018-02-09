#include "pc.h"

#include <QVBoxLayout>

pc::pc(int x, int y, MainServer* MSrv, QWidget *parent) : QWidget(parent)
{
	img = new QLabel(this);
	img->setPixmap(QPixmap(":/images/pc logo/PC.png"));
	img->setScaledContents(true);
	img->setMinimumHeight(48);
	img->setMaximumHeight(48);
	img->setMinimumWidth(48);
	img->setMaximumWidth(48);

	name = new QLabel(this);
	name->setText("PC name");
	name->setWordWrap(true);
	name->setAlignment(Qt::AlignHCenter);
	name->setAlignment(Qt::AlignTop);
	name->setMinimumHeight(16);
	name->setMaximumHeight(48);
	name->setMinimumWidth(48);
	name->setMaximumWidth(48);

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(img);
	layout->addWidget(name);
	layout->setMargin(0);
	layout->setSpacing(5);

	this->setLayout(layout);
	this->setGeometry(x, y, 48, 69);
	this->show();
	MServ = MSrv;
}

void pc::mouseMoveEvent(QMouseEvent*)
{
	drag = true;
	int X = this->cursor().pos().x() - MServ->geometry().x();
	int Y = this->cursor().pos().y() - MServ->geometry().y() - 20;

	name->setText(QString::number(X) + " " + QString::number(Y));
	this->setGeometry(X - lx, Y - ly, 48, 101);
}

pc::~pc() {
	delete img;
	delete name;
	MServ = NULL;
}

void pc::mousePressEvent(QMouseEvent *)
{
	drag = false;
	lx = this->cursor().pos().x() - MServ->geometry().x() - this->geometry().x();
	ly = this->cursor().pos().y() - MServ->geometry().y() - 20 - this->geometry().y();
}

void pc::mouseReleaseEvent(QMouseEvent *)
{
	if (!drag) {
		// settings pc
		name->setText("settings");
	}
}
