#ifndef TARGETSETTINGSDLG_H
#define TARGETSETTINGSDLG_H

#include <QDialog>
#include "ui_TargetSettingsDlg.h"

class TargetSettingsDlg : public QDialog
{
	Q_OBJECT

public:
	TargetSettingsDlg(QWidget *parent = 0);
	~TargetSettingsDlg();

public slots:

	void accept();
	void reject();

private:
	Ui::TargetSettingsDlg ui;
};

#endif // TARGETSETTINGSDLG_H
