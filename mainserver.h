#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>

namespace Ui {
	class MainServer;
}

class MainServer : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainServer(QWidget *parent = 0);
	~MainServer();
	Ui::MainServer* ui;

private:
	QTcpServer* mTcpServer;
	QTcpSocket* mTcpSocket;
	quint16 iNextBlockSize;

	void sendToClient(QTcpSocket* pSocket, const QString& str);

public slots:
	void slotNewConnection();
	void slotServerRead();
	void slotClientDisconnected();

protected:

};

#endif // MAINSERVER_H
