#ifndef WS_H
#define WS_H

#include "mainserver.h"

#include <QWidget>

class MainServer;
class WS : public QWidget
{
	Q_OBJECT
public:
	explicit WS(MainServer* MSrv, QWidget *parent = 0);
	~WS();
	pc* addPc(pcData& data);

private:
	MainServer* MServ;
	bool add;

protected:
	virtual void mouseReleaseEvent(QMouseEvent*);

signals:
	void SetChecked(bool);
	void newPC(pcData&);

public slots:
	void slotToggleBtn(bool);
};

#endif // WS_H
