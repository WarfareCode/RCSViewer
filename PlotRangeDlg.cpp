#include "PlotRangeDlg.h"

PlotRangeDlg::PlotRangeDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	ui.lineEdit->setText("60");
	ui.lineEdit_2->setText("20");

	QIntValidator* pValidator = new QIntValidator;
	pValidator->setRange(0, 100);
	ui.lineEdit->setValidator(pValidator);

	QIntValidator* pValidator2 = new QIntValidator;
	pValidator2->setRange(0, 100);
	ui.lineEdit_2->setValidator(pValidator2);

}

PlotRangeDlg::~PlotRangeDlg()
{

}

int PlotRangeDlg::GetMin()
{
	return ui.lineEdit_2->text().toInt();
}

int PlotRangeDlg::GetMax()
{
	return ui.lineEdit->text().toInt();
}