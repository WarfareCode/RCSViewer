#ifndef RADARBEAMROTATEDLG_H
#define RADARBEAMROTATEDLG_H

#include <QDialog>
#include "ui_RadarBeamRotateDlg.h"

class RadarBeamRotateDlg : public QDialog
{
	Q_OBJECT

public:
	RadarBeamRotateDlg(QWidget *parent = 0);
	~RadarBeamRotateDlg();

public slots:

	void accept();
	void reject();

private:
	Ui::RadarBeamRotateDlg ui;
};

#endif // RADARBEAMROTATEDLG_H
