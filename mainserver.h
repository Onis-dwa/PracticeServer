#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QGridLayout>

namespace Ui {
	class MainServer;
}

struct pcData;
class WS;
class pc;
class Settings;
class MainServer : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainServer(QWidget *parent = 0);
	~MainServer();
	Ui::MainServer* ui;

	void logfile(const QString& str);
	bool unsave;

private:
	QTcpServer* mTcpServer;
	QTcpSocket* mTcpSocket;
	quint16 iNextBlockSize;
	Settings* winset;

	QList<pc*>* pclist;

	WS* ws;
	QGridLayout* layout;
	QFile* flog;
	QFile* fsettings;
	QFile* fpcs;

	uint loadSettings();
	void loadpcs();

protected:
	virtual void closeEvent(QCloseEvent *event);

public slots:
	void slotNewConnection();

	void saveFile(bool);
	void menuSettings(bool);
	void settingsChanged(quint16, QString, QString);
	void newPC(pcData&);
};

struct pcData
{
	int x;
	int y;
	MainServer* MServ;
	QString Name;
	QString IP;
};

#endif // MAINSERVER_H
