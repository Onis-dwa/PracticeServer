#ifndef WS_H
#define WS_H

#include "mainserver.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class WS : public QWidget
{
	Q_OBJECT
public:
	explicit WS(MainServer* MSrv, QWidget *parent = 0);
	~WS();

private:
	QLabel* posx;
	QLabel* posy;
	QPushButton* addbtn;
	MainServer* MServ;
	bool add;
	int X;
	int Y;

protected:
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);

signals:

public slots:
	void slotToggleBtn(bool);
};

#endif // WS_H
