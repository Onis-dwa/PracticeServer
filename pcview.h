#ifndef PCVIEW_H
#define PCVIEW_H

#include "pcset.h"
#include "pc.h"
#include "graph.h"

#include <QDialog>

namespace Ui {
class pcview;
}
struct staticInfo;
struct dynamicInfo;
class pcview : public QDialog
{
	Q_OBJECT

public:
    explicit pcview(const QString& name, QWidget *parent = 0);
	~pcview();

	void setData(const QString& name, const QString& ip);
	void setData(staticInfo stI);
	void setData(dynamicInfo dnI);

    void setrand(QString rnd);
    QString GetStatic();
    QString GetDynaminc();

private:
	Ui::pcview *ui;
    PcSet* set;

    Graph* cpu;
    Graph* gpu;
    Graph* ram;

protected:
    virtual void closeEvent(QCloseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void resizeEvent(QResizeEvent*);

signals:
    void RemoteRun(bool);
    void HeadChanged();
	void PcSetChanged(QString, QString);

public slots:
	void ShowSettings(bool);
    void hideall(bool);
};

#endif // PCVIEW_H
