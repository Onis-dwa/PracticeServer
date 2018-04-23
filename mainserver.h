#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>

namespace Ui {
	class MainServer;
}

struct pcData;
class WS;
class dep;
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
	void moveWS(int x, int y);
	int tryMove(pc* obj, int& newx, int& newy);
	int tryDepMove(dep* obj, int& newx, int& newy);
	WS* GetWS();

	bool unsave;

private:
	QTcpServer* mTcpServer;
	QTcpSocket* mTcpSocket;

	quint16 iNextBlockSize;
	quint16 Serverport;

	Settings* winset;
	WS* ws;
	QList<pc*>* pclist;
	QList<dep*>* deplist;

	QFile* flog;
	QFile* fsettings;
	QFile* fdata;

	void loadSettings();
	void loadData();
	void resizeEvent(QResizeEvent* event);

protected:
	virtual void closeEvent(QCloseEvent *event);

public slots:
	void slotNewConnection();

	void saveFile(bool);
	void menuSettings(bool);
	void settingsChanged(quint16, QString, QString);
	void newPC(int x, int y);
	void newDep(int x, int y);
	void AddPc(bool);
	void AddDep(bool);
	void GoHome(bool);
};

struct pcData
{
	int x;
	int y;
	QString Name;
	QString IP;


	friend inline QTextStream& operator >>(QTextStream& t, pcData& p)
	{
		return t >> p.x >> p.y >> p.Name >> p.IP;
	}
	friend inline QTextStream& operator <<(QTextStream& t, pcData& p)
	{
		return t << p.x << " " << p.y << "\n" << p.Name << "\n" << p.IP;
	}
};
struct depData
{
	int x;
	int y;
	QString Name;
	int width;
	int heigth;


	friend inline QTextStream& operator >>(QTextStream& t, depData& p)
	{
		return t >> p.x >> p.y >> p.Name >> p.width >> p.heigth;
	}
	friend inline QTextStream& operator <<(QTextStream& t, depData& p)
	{
		return t << p.x << " " << p.y << "\n" << p.Name << "\n" << p.width << " " << p.heigth;
	}
};

#endif // MAINSERVER_H
