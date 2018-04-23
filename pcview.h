#ifndef PCVIEW_H
#define PCVIEW_H

#include "pcset.h"
#include "pc.h"

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
	explicit pcview(QWidget *parent = 0);
	~pcview();

	void setData(const QString& name, const QString& ip);
	void setData(staticInfo stI);
	void setData(dynamicInfo dnI);

private:
	Ui::pcview *ui;
	PcSet* set;

signals:
	void PcSetChanged(QString, QString);

public slots:
	void ShowSettings(bool);
};

#endif // PCVIEW_H
