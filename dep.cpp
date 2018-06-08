#include "dep.h"

#include <QMouseEvent>

// public
dep::dep(depData& data, QFrame *parent, MainServer* MServer) : QFrame(parent)
{
	QColor grayA(83, 83, 83, 200);
	QPalette back;
	back.setBrush(QPalette::Background, grayA);
	this->setAutoFillBackground(true);
    this->setPalette(back);
	this->setFrameShape(Shape::Box);
	this->setFrameShadow(Shadow::Plain);
	this->setLineWidth(2);
	this->setMidLineWidth(1);
	this->setMouseTracking(true);
	this->setGeometry(data.x, data.y, data.width, data.heigth);

	MServ = MServer;
	IsDrag = false;
	IsResize = false;
	BorderSize = 5;
	last = true;

	name = new QLineEdit(this);
	name->setStyleSheet("QLineEdit {"
						"background-color: #535353;"
						"border-style: solid;"
						"border-color: black;"
						"border-width: 2px;"
						"color: white};");
	name->setAlignment(Qt::AlignHCenter);
	name->setGeometry(BorderSize, BorderSize, this->width() - (BorderSize << 1), 21);
	name->setText(data.Name);
    name->setFont(QFont("MS Shell Dlg 2", 11));

	unsaved = new QLabel(this);
	unsaved->setPixmap(QPixmap(":/res/resources/Unsaved.png"));
	unsaved->setScaledContents(true);
	unsaved->raise();
	unsaved->setGeometry(this->width() - 16, 0, 16, 16);

	this->show();
}
void dep::DropDown(bool state)
{
	if (state && !last)
	{
		QColor darkblueA(70, 80, 90, 200);
		QPalette back;
		back.setBrush(QPalette::Background, darkblueA);
		this->setPalette(back);
		last = true;
	}
	else if (!state && last)
	{
		QColor grayA(83, 83, 83, 200);
		QPalette back;
		back.setBrush(QPalette::Background, grayA);
		this->setPalette(back);
		last = false;
	}
}
depData *dep::GetData()
{
	depData* dt = new depData();
	dt->x = this->x();
	dt->y = this->y();
	dt->Name = name->text();
	dt->width = this->width();
	dt->heigth = this->height();

	return dt;
}

// protected
void dep::mousePressEvent(QMouseEvent* m)
{
	switch (DirResize) {
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		IsResize = true;
		DragX = this->cursor().pos().x();
		DragY = this->cursor().pos().y();
		break;
	}

	if (m->button() & Qt::RightButton)
	{
		IsDrag = true;
		DragX = this->cursor().pos().x();
		DragY = this->cursor().pos().y();

		this->setCursor(Qt::OpenHandCursor);
	}
}
void dep::mouseMoveEvent(QMouseEvent* m)
{
	if (IsResize)
	{
		int TX = DragX - this->cursor().pos().x();
		int TY = DragY - this->cursor().pos().y();

//		int maxW;
//		int maxH;
		// ограничение размеров хз как

		switch (DirResize) {
		case 1:
			this->setGeometry(this->x() - TX,
							  this->y() - TY,
							  this->width() + TX,
							  this->height() + TY);
			break;
		case 2:
			this->setGeometry(this->x() - TX,
							  this->y(),
							  this->width() + TX,
							  this->height() - TY);
			break;
		case 3:
			this->setGeometry(this->x(),
							  this->y() - TY,
							  this->width() - TX,
							  this->height() + TY);
			break;
		case 4:
			this->setGeometry(this->x(),
							  this->y(),
							  this->width() - TX,
							  this->height() - TY);
			break;
		case 5:
			this->setGeometry(this->x() - TX,
							  this->y(),
							  this->width() + TX,
							  this->height());
			break;
		case 6:
			this->setGeometry(this->x(),
							  this->y(),
							  this->width() - TX,
							  this->height());
			break;
		case 7:
			this->setGeometry(this->x(),
							  this->y() - TY,
							  this->width(),
							  this->height() + TY);
			break;
		case 8:
			this->setGeometry(this->x(),
							  this->y(),
							  this->width(),
							  this->height() - TY);
			break;
		}
		name->setGeometry(BorderSize, BorderSize, this->width() - (BorderSize << 1), 21);
		unsaved->move(this->width() - 16, 0);
		DragX = this->cursor().pos().x();
		DragY = this->cursor().pos().y();
		MServ->unsave = true;
		unsaved->show();
		return;
	}
	else
	{
		// left top
		if (m->x() < BorderSize &&
			m->y() < BorderSize)
		{
			DirResize = 1;
			this->setCursor(Qt::SizeFDiagCursor);
		}
		// left bottom
		else if (m->x() < BorderSize &&
				 m->y() > (this->height() - BorderSize))
		{
			DirResize = 2;
			this->setCursor(Qt::SizeBDiagCursor);
		}
		// right top
		else if (m->x() > (this->width() - BorderSize) &&
				 m->y() < BorderSize)
		{
			DirResize = 3;
			this->setCursor(Qt::SizeBDiagCursor);
		}
		// right bottom
		else if (m->x() > (this->width() - BorderSize) &&
				 m->y() > (this->height() - BorderSize))
		{
			DirResize = 4;
			this->setCursor(Qt::SizeFDiagCursor);
		}
		// left
		else if (m->x() < BorderSize)
		{
			DirResize = 5;
			this->setCursor(Qt::SizeHorCursor);
		}
		// right
		else if (m->x() > (this->width() - BorderSize))
		{
			DirResize = 6;
			this->setCursor(Qt::SizeHorCursor);
		}
		// top
		else if (m->y() < BorderSize)
		{
			DirResize = 7;
			this->setCursor(Qt::SizeVerCursor);
		}
		// bottom
		else if (m->y() > (this->height() - BorderSize))
		{
			DirResize = 8;
			this->setCursor(Qt::SizeVerCursor);
		}
		else
		{
			DirResize = 0;
			this->setCursor(Qt::ArrowCursor);
		}
	}

	if (IsDrag)
	{
		int TX = this->x() - (DragX - this->cursor().pos().x());
		int TY = this->y() - (DragY - this->cursor().pos().y());
		int res = MServ->tryDepMove(this, TX, TY);

		switch (res) {
		case 1:
			this->move(TX, TY);
			DragX = this->cursor().pos().x();
			DragY = this->cursor().pos().y();
			break;
		case 2:
			this->move(TX, this->y());
			DragX = this->cursor().pos().x();
			break;
		case 3:
			this->move(this->x(), TY);
			DragY = this->cursor().pos().y();
			break;
		}

		MServ->unsave = true;
		unsaved->show();
	}
}
void dep::mouseReleaseEvent(QMouseEvent*)
{
	IsResize = false;
	DirResize = 0;
	IsDrag = false;
	this->setCursor(Qt::ArrowCursor);
}
