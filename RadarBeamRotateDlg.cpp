#include "RadarBeamRotateDlg.h"
#include "DataManager.h"

RadarBeamRotateDlg::RadarBeamRotateDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	DataManager* pDataManager = DataManager::Instance();

	double dOffsetX, dOffsetY, dOffsetZ, dRotateX
		, dRotateY, dRotateZ, dScale;

	pDataManager->GetRadarBeamLocalMatrix(dOffsetX, dOffsetY, dOffsetZ, dRotateX
		, dRotateY, dRotateZ, dScale);

	ui.lineEdit_OffsetX->setText(QString::number(dOffsetX));
	ui.lineEdit_OffsetY->setText(QString::number(dOffsetY));
	ui.lineEdit_OffsetZ->setText(QString::number(dOffsetZ));
	ui.lineEdit_RotateX->setText(QString::number(dRotateX));
	ui.lineEdit_RotateY->setText(QString::number(dRotateY));
	ui.lineEdit_RotateZ->setText(QString::number(dRotateZ));
	ui.lineEdit_Scale->setText(QString::number(dScale));

	ui.lineEdit_OffsetX->setValidator(new QDoubleValidator);
	ui.lineEdit_OffsetY->setValidator(new QDoubleValidator);
	ui.lineEdit_OffsetZ->setValidator(new QDoubleValidator);
	ui.lineEdit_RotateX->setValidator(new QDoubleValidator);
	ui.lineEdit_RotateY->setValidator(new QDoubleValidator);
	ui.lineEdit_RotateZ->setValidator(new QDoubleValidator);
	ui.lineEdit_Scale->setValidator(new QDoubleValidator);

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

RadarBeamRotateDlg::~RadarBeamRotateDlg()
{

}

void RadarBeamRotateDlg::accept()
{
	double dOffsetX = ui.lineEdit_OffsetX->text().toDouble();
	double dOffsetY = ui.lineEdit_OffsetY->text().toDouble();
	double dOffsetZ = ui.lineEdit_OffsetZ->text().toDouble();
	double dRotateX = ui.lineEdit_RotateX->text().toDouble();
	double dRotateY = ui.lineEdit_RotateY->text().toDouble();
	double dRotateZ = ui.lineEdit_RotateZ->text().toDouble();
	double dScale = ui.lineEdit_Scale->text().toDouble();

	DataManager* pDataManager = DataManager::Instance();
	pDataManager->SetRadarBeamLocalMatrix(dOffsetX, dOffsetY, dOffsetZ, dRotateX, dRotateY, dRotateZ, dScale);

	QDialog::accept();
}

void RadarBeamRotateDlg::reject()
{
	QDialog::reject();
}