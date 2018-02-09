#include "ws.h"
#include "pc.h"

#include <QMouseEvent>

WS::WS(MainServer* MSrv, QWidget *parent) : QWidget(parent)
{
	posx = new QLabel(this);
	posx->setGeometry(5,5, 50, 13);

	posy = new QLabel(this);
	posy->setGeometry(5,23, 50, 13);

	addbtn = new QPushButton(this);
	addbtn->setGeometry(5, 41, 50, 21);
	addbtn->setText("Add PC");
	addbtn->setCheckable(true);
	addbtn->setChecked(false);

	connect(addbtn, SIGNAL(toggled(bool)), SLOT(slotToggleBtn(bool)));

	add = false;
	MServ = MSrv;
	this->setMouseTracking(true);
	this->show();
}

WS::~WS()
{
	delete posx;
	delete posy;
	delete addbtn;
	MServ = NULL;
}

void WS::mouseMoveEvent(QMouseEvent*)
{
	X = this->cursor().pos().x() - MServ->x() - 8;
	Y = this->cursor().pos().y() - MServ->y() - 31;
	posx->setText(QString::number(X));
	posy->setText(QString::number(Y));
}

void WS::mouseReleaseEvent(QMouseEvent* m)
{
	if (add && (m->button() & Qt::RightButton)) {
		// add pc in coordinate
		pc* npc = new pc(X, Y, MServ, this);
		add = false;
		addbtn->setChecked(false);
	}
}

void WS::slotToggleBtn(bool)
{
	if (add) {
		add = false;
		this->setCursor(QCursor(Qt::ArrowCursor));
	}
	else {
		add = true;
		this->setCursor(QCursor(Qt::CrossCursor));
	}
}

