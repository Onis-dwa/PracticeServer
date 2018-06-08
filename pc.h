#ifndef PC_H
#define PC_H

#include "mainserver.h"
#include "pcview.h"

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QTime>

class dep;
class pcview;
class pc : public QWidget
{
	Q_OBJECT
public:
	explicit pc(const pcData& data, QWidget *parent, MainServer* MServer);
	~pc();

	void Connect(QTcpSocket* skt);
	pcData* GetData();
	void SetPixmap(QString);
    void Remove(bool);
    void RunPing();
    void Rename();

    bool isConnected;
    bool remove;
	dep* Dep;
    QLabel* unsaved;

private:
	QLabel* img;
	QLabel* name;
	QVBoxLayout* layout;

	MainServer* MServ;
	QTcpSocket* Socket;
	pcview* PcView;

	bool drag;
	bool view;
	int DragX;
	int DragY;
	quint8 NBlockSize;
    QString ip;

    QTime timer;
    bool headerchanged;
    bool unsaved_data;
    bool bload;
    bool rename;

    void Load();
    void Save();
    QString ParseTime();
    QString oldname;

protected:
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);

private slots:
    void GetRand(bool);
    void ReadClient();
    void Disconnect();
    void PcDataSet(QString,QString);
	void PcSetChanged(QString, QString);
    void HeadChanged();
};
struct staticInfo
{
	QString PCName;
	QString OS;
	QString Bit;
	QString CPU;
	QString GPU;
	QString RAM;

    friend inline QTextStream& operator >>(QTextStream& t, staticInfo& p)
    {
        p.PCName = t.readLine().replace("	", "");
        p.OS = t.readLine().replace("	", "");
        p.Bit = t.readLine().replace("	", "");
        p.CPU = t.readLine().replace("	", "");
        p.GPU = t.readLine().replace("	", "");
        p.RAM = t.readLine().replace("	", "");

        return t;
    }
	friend inline QDataStream& operator >>(QDataStream& t, staticInfo& p)
	{
		return t >> p.PCName >> p.OS >> p.Bit >> p.CPU >> p.GPU >> p.RAM;
	}
	friend inline QDataStream& operator <<(QDataStream& t, staticInfo& p)
	{
		return t << p.PCName << "\n"
				 << p.OS	 << "\n"
				 << p.Bit	 << "\n"
				 << p.CPU	 << "\n"
				 << p.GPU	 << "\n"
				 << p.RAM	 << "\n";
	}
};
struct dynamicInfo
{
	double CPU;
	double GPU;
	double RAM;

	friend inline QDataStream& operator >>(QDataStream& t, dynamicInfo& p)
	{
		return t >> p.CPU >> p.GPU >> p.RAM;
	}
	friend inline QDataStream& operator <<(QDataStream& t, dynamicInfo& p)
	{
		return t << p.CPU << "\n"
				 << p.GPU << "\n"
				 << p.RAM << "\n";
	}
};

#endif // PC_H
