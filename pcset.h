#ifndef PCSET_H
#define PCSET_H

#include <QDialog>

namespace Ui {
	class PcSet;
}

class PcSet : public QDialog
{
	Q_OBJECT

public:
	explicit PcSet(QWidget *parent = 0);
	~PcSet();

	void setData(const QString& name, const QString& ip);

private:
	Ui::PcSet* ui;
	QString Name;
	QString IP;

signals:
	void PcSetChanged(QString, QString);

private slots:
	void accepted();
};

#endif // PCSET_H
