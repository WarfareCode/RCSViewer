#include "TargetSettingsDlg.h"
#include "DataManager.h"
#include <QtGui/QDoubleValidator>

TargetSettingsDlg::TargetSettingsDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	DataManager* pDataManager = DataManager::Instance();
	double dLon, dLat, dHeight, dRotateX, dRotateY, dRotateZ, dScale;
	pDataManager->GetTargetPara(dLon, dLat, dHeight, dRotateX, dRotateY, dRotateZ, dScale);

	ui.lineEdit_Lon->setValidator(new QDoubleValidator);
	ui.lineEdit_Lon->setText(QString::number(dLon));

	ui.lineEdit_Lat->setValidator(new QDoubleValidator);
	ui.lineEdit_Lat->setText(QString::number(dLat));

	ui.lineEdit_H->setValidator(new QDoubleValidator);
	ui.lineEdit_H->setText(QString::number(dHeight));

	ui.lineEdit_RotateX->setValidator(new QDoubleValidator);
	ui.lineEdit_RotateX->setText(QString::number(dRotateX));

	ui.lineEdit_RotateY->setValidator(new QDoubleValidator);
	ui.lineEdit_RotateY->setText(QString::number(dRotateY));

	ui.lineEdit_RotateZ->setValidator(new QDoubleValidator);
	ui.lineEdit_RotateZ->setText(QString::number(dRotateZ));

	ui.lineEdit_Scale->setValidator(new QDoubleValidator);
	ui.lineEdit_Scale->setText(QString::number(dScale));

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

TargetSettingsDlg::~TargetSettingsDlg()
{

}

void TargetSettingsDlg::accept()
{
	double dLon = ui.lineEdit_Lon->text().toDouble();

	double dLat = ui.lineEdit_Lat->text().toDouble();

	double dHeight = ui.lineEdit_H->text().toDouble();

	double dRotateX = ui.lineEdit_RotateX->text().toDouble();

	double dRotateY = ui.lineEdit_RotateY->text().toDouble();

	double dRotateZ = ui.lineEdit_RotateZ->text().toDouble();

	double dScale = ui.lineEdit_Scale->text().toDouble();

	DataManager* pDataManager = DataManager::Instance();
	pDataManager->SetTargetPara(dLon, dLat, dHeight, dRotateX, dRotateY, dRotateZ, dScale);

	QDialog::accept();
}

void TargetSettingsDlg::reject()
{
	QDialog::reject();
}