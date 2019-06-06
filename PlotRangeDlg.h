#ifndef PLOTRANGEDLG_H
#define PLOTRANGEDLG_H

#include <QDialog>
#include "ui_PlotRangeDlg.h"

class PlotRangeDlg : public QDialog
{
	Q_OBJECT

public:
	PlotRangeDlg(QWidget *parent = 0);
	~PlotRangeDlg();

	int GetMin();

	int GetMax();

private:
	Ui::PlotRangeDlg ui;
};

#endif // PLOTRANGEDLG_H
