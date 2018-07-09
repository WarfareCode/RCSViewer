#include "AeroRotateDlg.h"
#include "DataManager.h"

AeroRotateDlg::AeroRotateDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	DataManager* pDataManager = DataManager::Instance();
	float x, y, z;
	pDataManager->GetAerocraftRotate(x, y, z);

	ui.lineEdit_X->setValidator(new QDoubleValidator);
	ui.lineEdit_Y->setValidator(new QDoubleValidator);
	ui.lineEdit_Z->setValidator(new QDoubleValidator);
	ui.lineEdit_Scale->setValidator(new QDoubleValidator);

	ui.lineEdit_X->setText(QString::number(x));
	ui.lineEdit_Y->setText(QString::number(y));
	ui.lineEdit_Z->setText(QString::number(z));
	ui.lineEdit_Scale->setText(QString::number(pDataManager->GetAerocraftScale()));

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

AeroRotateDlg::~AeroRotateDlg()
{

}

void AeroRotateDlg::accept()
{
	if (ui.lineEdit_X->text().isEmpty())
		return;

	if (ui.lineEdit_Y->text().isEmpty())
		return;

	if (ui.lineEdit_Z->text().isEmpty())
		return;

	if (ui.lineEdit_Scale->text().isEmpty())
		return;

	double dRotateX = ui.lineEdit_X->text().toDouble();
	double dRotateY = ui.lineEdit_Y->text().toDouble();
	double dRotateZ = ui.lineEdit_Z->text().toDouble();
	double dRotateScale = ui.lineEdit_Scale->text().toDouble();

	DataManager* pDataManager = DataManager::Instance();
	pDataManager->SetAerocraftRotate(dRotateX, dRotateY, dRotateZ);
	double dOldScale = pDataManager->GetAerocraftScale();
	if (dOldScale != dRotateScale)
	{
		pDataManager->SetAerocraftScale(dRotateScale);
	}

	QDialog::accept();
}

void AeroRotateDlg::reject()
{
	QDialog::reject();
}
