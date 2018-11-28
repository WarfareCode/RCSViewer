#ifndef DATEINPUTDLG_H
#define DATEINPUTDLG_H

#include <QDialog>
#include "ui_DateInputDlg.h"

class DateInputDlg : public QDialog
{
	Q_OBJECT

public:
	DateInputDlg(QWidget *parent = 0);
	~DateInputDlg();

	QDate date();

public slots:

	void accept();
	void reject();
	 
private:
	Ui::DateInputDlg ui;
};

#endif // DATEINPUTDLG_H
