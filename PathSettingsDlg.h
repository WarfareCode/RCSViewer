#ifndef PATHSETTINGSDLG_H
#define PATHSETTINGSDLG_H

#include <QDialog>
#include "ui_PathSettingsDlg.h"

class PathSettingsDlg : public QDialog
{
	Q_OBJECT

public:
	PathSettingsDlg(QWidget *parent = 0);
	~PathSettingsDlg();

public slots:

	void accept();
	void reject();


private:
	Ui::PathSettingsDlg ui;
};

#endif // PATHSETTINGSDLG_H
