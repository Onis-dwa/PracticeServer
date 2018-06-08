#include "ws.h"
#include "pc.h"

#include <QMouseEvent>

// public
WS::WS(MainServer* MServer, QWidget *parent) : QWidget(parent)
{
	QPalette palette;
	palette.setBrush(QPalette::Background, QPixmap(":/res/resources/sota.png"));
	this->setAutoFillBackground(true);
	this->setPalette(palette);

	this->setMouseTracking(true);

	MServ = MServer;
	isDrag = false;
	isAddPC = false;
	isAddDep = false;
	this->show();
}
WS::~WS()
{

}
void WS::toggleAddPC(bool state)
{
	if (state) {
		isAddPC = true;
		this->setCursor(QCursor(Qt::CrossCursor));
	}
	else {
		isAddPC = false;
		this->setCursor(QCursor(Qt::ArrowCursor));
	}
}
void WS::toggleAddDep(bool state)
{
	if (state) {
		isAddDep = true;
		this->setCursor(QCursor(Qt::CrossCursor));
	}
	else {
		isAddDep = false;
		this->setCursor(QCursor(Qt::ArrowCursor));
	}
}


// protected
void WS::mousePressEvent(QMouseEvent* m)
{
	if (m->button() & Qt::RightButton)
	{
		this->setCursor(QCursor(Qt::OpenHandCursor));
		isDrag = true;
	}
	DragX = this->cursor().pos().x();
	DragY = this->cursor().pos().y();
}
void WS::mouseMoveEvent(QMouseEvent*)
{
	if (isDrag)
	{
		MServ->moveWS(DragX - this->cursor().pos().x(), DragY - this->cursor().pos().y());

		DragX = this->cursor().pos().x();
		DragY = this->cursor().pos().y();
	}
}
void WS::mouseReleaseEvent(QMouseEvent* m)
{
	if (!isDrag && isAddPC && (m->button() & Qt::LeftButton)) {
		MServ->newPC(m->x(), m->y());
	}
	else if (!isDrag && isAddDep && (m->button() & Qt::LeftButton)) {
		MServ->newDep(m->x(), m->y());
	}
	else
	{
		this->setCursor(QCursor(Qt::ArrowCursor));
		isDrag = false;
	}
}
