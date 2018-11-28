#ifndef ADDDIRDLG_H
#define ADDDIRDLG_H

#include <QDialog>
#include "ui_AddDirDlg.h"

class AddDirDlg : public QDialog
{
	Q_OBJECT

public:
	AddDirDlg(QWidget *parent = 0);
	~AddDirDlg();

	QString strDir();

public slots:

	void accept();
	void reject();

private:
	Ui::AddDirDlg ui;
};

#endif // ADDDIRDLG_H
