#ifndef NEWPROJECTDLG_H
#define NEWPROJECTDLG_H

#include <QDialog>
#include "ui_NewProjectDlg.h"

class NewProjectDlg : public QDialog
{
	Q_OBJECT

public:
	NewProjectDlg(QWidget *parent = 0);
	~NewProjectDlg();

	QString projectDir();
	QString projectName();

public slots:

	void accept();
	void reject();

	void slotSetPrjDir();

private:
	Ui::NewProjectDlg ui;
};

#endif // NEWPROJECTDLG_H
