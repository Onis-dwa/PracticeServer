#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

namespace Ui {
	class Settings;
}

class Settings : public QDialog
{
	Q_OBJECT

public:
	explicit Settings(QWidget *parent = 0);
	~Settings();
	Ui::Settings* ui;

	void setData(const QString& ip, const quint16& port, const QString& nameLogFile, const QString& namePcsFile);

private:
	QString Ip;
	quint16 Port;
	QString Nlf;
	QString Npf;

signals:
	void settingsChanged(quint16, QString, QString);

private slots:
	void accepted();
};

#endif // SETTINGS_H
