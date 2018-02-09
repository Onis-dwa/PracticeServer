#ifndef PC_H
#define PC_H

#include "mainserver.h"
#include <QWidget>
#include <QLabel>

class pc : public QWidget
{
	Q_OBJECT
public:
	explicit pc(int x, int y, MainServer* MSrv, QWidget *parent = 0);
	~pc();

private:
	QLabel* img;
	QLabel* name;
	MainServer* MServ;

	bool drag;
	int lx;
	int ly;

protected:
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);

};

#endif // PC_H
