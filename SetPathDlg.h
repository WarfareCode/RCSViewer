#ifndef SETPATHDLG_H
#define SETPATHDLG_H

#include <QDialog>
#include "ui_SetPathDlg.h"

class SetPathDlg : public QDialog
{
	Q_OBJECT

public:
	SetPathDlg(QString strPath, QWidget *parent = 0);
	~SetPathDlg();

public:

	QString path();

public slots:

	void slotSetPath();

private:
	Ui::SetPathDlg ui;
};

#endif // SETPATHDLG_H
