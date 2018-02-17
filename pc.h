#ifndef PC_H
#define PC_H

#include "mainserver.h"
#include "pcset.h"

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class pc : public QWidget
{
	Q_OBJECT
public:
	explicit pc(const pcData& data, QWidget *parent = 0);
	~pc();

	void Connect(QTcpSocket* skt);
	pcData* GetData();
	void SetPixmap(QString);
	bool isActive;

private:
	QLabel* img;
	QLabel* name;
	QVBoxLayout* layout;

	MainServer* MServ;
	QTcpSocket* Socket;
	PcSet* set;

	bool drag;
	int lx;
	int ly;
	quint8 NBlockSize;
	QString ip;

	void SendClient(const QString& str);
//	void ping();

protected:
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);

private slots:
	void ReadClient();
	void Disconnect();
	void PcSetChanged(QString, QString);
};

#endif // PC_H
