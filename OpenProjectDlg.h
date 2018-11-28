#ifndef OPENPROJECTDLG_H
#define OPENPROJECTDLG_H

#include <QDialog>
#include "ui_OpenProjectDlg.h"

class OpenProjectDlg : public QDialog
{
	Q_OBJECT

public:
	OpenProjectDlg(QWidget *parent = 0);
	~OpenProjectDlg();

private:
	Ui::OpenProjectDlg ui;
};

#endif // OPENPROJECTDLG_H
