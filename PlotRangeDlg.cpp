#include "PlotRangeDlg.h"
#include "PlotSettings.h"

PlotRangeDlg::PlotRangeDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	setWindowTitle(QString::fromLocal8Bit("ÉèÖÃ"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowIcon(QIcon());

	int nHMin, nHMax, nVMin, nVMax;
	PlotSettings* pSettings = PlotSettings::Instance();
	pSettings->getHMinMax(nHMin, nHMax);
	pSettings->getVMinMax(nVMin, nVMax);

	ui.lineEdit_HLabel->setText(pSettings->hLabel());
	ui.lineEdit_VLabel->setText(pSettings->vLabel());

	ui.lineEdit_VMax->setText(QString::number(nVMax));
	ui.lineEdit_VMin->setText(QString::number(nVMin));

	QIntValidator* pValidator = new QIntValidator;
	pValidator->setRange(0, 100);
	ui.lineEdit_VMax->setValidator(pValidator);

	QIntValidator* pValidator2 = new QIntValidator;
	pValidator2->setRange(0, 100);
	ui.lineEdit_VMin->setValidator(pValidator2);

	ui.lineEdit_HMax->setText(QString::number(nHMax));
	ui.lineEdit_HMin->setText(QString::number(nHMin));

	QIntValidator* pValidator3 = new QIntValidator;
	//pValidator3->setRange(0, 100);
	ui.lineEdit_HMax->setValidator(pValidator3);

	QIntValidator* pValidator4 = new QIntValidator;
	//pValidator4->setRange(0, 100);
	ui.lineEdit_HMin->setValidator(pValidator4);
}

PlotRangeDlg::~PlotRangeDlg()
{

}

int PlotRangeDlg::GetVMin()
{
	return ui.lineEdit_VMin->text().toInt();
}

int PlotRangeDlg::GetVMax()
{
	return ui.lineEdit_VMax->text().toInt();
}

int PlotRangeDlg::GetHMin()
{
	return ui.lineEdit_HMin->text().toInt();
}

int PlotRangeDlg::GetHMax()
{
	return ui.lineEdit_HMax->text().toInt();
}

QString PlotRangeDlg::hLabel()
{
	return ui.lineEdit_HLabel->text();
}

QString PlotRangeDlg::vLabel()
{
	return ui.lineEdit_VLabel->text();
}