#ifndef VALUEINPUTDLG_H
#define VALUEINPUTDLG_H

#include <QDialog>
#include "ui_ValueInputDlg.h"

class ValueInputDlg : public QDialog
{
	Q_OBJECT

public:
	ValueInputDlg(QString strFieldType, QWidget *parent = 0);
	~ValueInputDlg();

	QString editText();

public slots:

	void accept();
	void reject();

private:
	Ui::ValueInputDlg ui;
};

#endif // VALUEINPUTDLG_H
