#ifndef SQLSELECTDLG_H
#define SQLSELECTDLG_H

#include <QDialog>
#include <QJsonObject>
#include "ui_SQLSelectDlg.h"

class SQLSelectDlg : public QDialog
{
	Q_OBJECT

public:
	SQLSelectDlg(const QJsonObject& classObj, QWidget *parent = 0);
	~SQLSelectDlg();

	QString SQLString();

public slots:

	void slotFieldButtonClicked();

	void slotInputDate();

	void accept();
	void reject();

private:
	Ui::SQLSelectDlg ui;

	QJsonObject m_jsonObj;
};

#endif // SQLSELECTDLG_H
