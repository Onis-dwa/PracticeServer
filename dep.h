#ifndef DEP_H
#define DEP_H

#include "mainserver.h"
#include <QFrame>
#include <QLabel>
#include <QLineEdit>

class dep : public QFrame
{
	Q_OBJECT
public:
	explicit dep(depData& data, QFrame *parent, MainServer* MServer);

	void DropDown(bool);
	depData* GetData();

	QLabel* unsaved;
	QLineEdit* name;

private:
	MainServer* MServ;

	bool IsDrag;
	bool IsResize;
	bool last;
	int DirResize;
	int BorderSize;
	int DragX;
	int DragY;

protected:
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);
};

#endif // DEP_H
