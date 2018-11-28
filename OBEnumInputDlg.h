#ifndef OBENUMINPUTDLG_H
#define OBENUMINPUTDLG_H

#include <QDialog>
#include "ui_OBEnumInputDlg.h"

class OBEnumInputDlg : public QDialog
{
	Q_OBJECT

public:
	OBEnumInputDlg(QString strFieldType, QWidget *parent = 0);
	~OBEnumInputDlg();

	QString text();

public slots:

	void accept();
	void reject();

private:
	Ui::OBEnumInputDlg ui;
};

#endif // OBENUMINPUTDLG_H
