#ifndef WS_H
#define WS_H

#include "mainserver.h"
#include <QWidget>

class MainServer;
class WS : public QWidget
{
	Q_OBJECT
public:
	explicit WS(MainServer* MServer, QWidget *parent = 0);
	~WS();

	void toggleAddPC(bool);
	void toggleAddDep(bool);

private:
	MainServer* MServ;

	bool isDrag;
	bool isAddPC;
	bool isAddDep;

	int DragX;
	int DragY;

protected:
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);
};

#endif // WS_H
