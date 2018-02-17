#include "ws.h"
#include "pc.h"

#include <QMouseEvent>

// public
WS::WS(MainServer* MSrv, QWidget *parent) : QWidget(parent)
{
	add = false;
	MServ = MSrv;
	this->show();
}
WS::~WS()
{
	MServ = NULL;
}
pc* WS::addPc(pcData &data)
{
	return new pc(data, this);
}


// protected
void WS::mouseReleaseEvent(QMouseEvent* m)
{
	if (add && (m->button() & Qt::LeftButton)) {
		int X = this->cursor().pos().x() - MServ->x() - 8;
		int Y = this->cursor().pos().y() - MServ->y() - 52;

		pcData* dt = new pcData();
		dt->x = X;
		dt->y = Y;
		dt->MServ = MServ;
		dt->Name = "localhost";
		dt->IP = "127.0.0.1";

		this->newPC(*dt);
		this->slotToggleBtn(false);
	}
}

// public slots
void WS::slotToggleBtn(bool state)
{
	if (state) {
		add = true;
		this->setCursor(QCursor(Qt::CrossCursor));
		this->SetChecked(true);
	}
	else {
		add = false;
		this->setCursor(QCursor(Qt::ArrowCursor));
		this->SetChecked(false);
	}
}

