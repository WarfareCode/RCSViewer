#include "PathSettingsDlg.h"
#include "DataManager.h"

PathSettingsDlg::PathSettingsDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	DataManager* pDataManager = DataManager::Instance();

	osg::Vec3d circleCenter;
	double dRadius;
	double dCircleTime;
	pDataManager->GetCirclePathPara(circleCenter, dRadius, dCircleTime);

	osg::Vec3d startPoint;
	osg::Vec3d endPoint;
	double dLineTime;
	pDataManager->GetLinePathPara(startPoint, endPoint, dLineTime);

	ui.lineEdit_CenterX->setValidator(new QDoubleValidator);
	ui.lineEdit_CenterY->setValidator(new QDoubleValidator);
	ui.lineEdit_Height->setValidator(new QDoubleValidator);
	ui.lineEdit_Radius->setValidator(new QDoubleValidator);
	ui.lineEdit_CircleTime->setValidator(new QDoubleValidator);

	ui.lineEdit_StartX->setValidator(new QDoubleValidator);
	ui.lineEdit_StartY->setValidator(new QDoubleValidator);
	ui.lineEdit_StartHeight->setValidator(new QDoubleValidator);
	ui.lineEdit_EndX->setValidator(new QDoubleValidator);
	ui.lineEdit_EndY->setValidator(new QDoubleValidator);
	ui.lineEdit_EndHeight->setValidator(new QDoubleValidator);
	ui.lineEdit_LineTime->setValidator(new QDoubleValidator);

	ui.lineEdit_CenterX->setText(QString::number(circleCenter.x()));
	ui.lineEdit_CenterY->setText(QString::number(circleCenter.y()));
	ui.lineEdit_Height->setText(QString::number(circleCenter.z()));
	ui.lineEdit_Radius->setText(QString::number(dRadius));
	ui.lineEdit_CircleTime->setText(QString::number(dCircleTime));

	ui.lineEdit_StartX->setText(QString::number(startPoint.x()));
	ui.lineEdit_StartY->setText(QString::number(startPoint.y()));
	ui.lineEdit_StartHeight->setText(QString::number(startPoint.z()));
	ui.lineEdit_EndX->setText(QString::number(endPoint.x()));
	ui.lineEdit_EndY->setText(QString::number(endPoint.y()));
	ui.lineEdit_EndHeight->setText(QString::number(endPoint.z()));
	ui.lineEdit_LineTime->setText(QString::number(dLineTime));

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
}

PathSettingsDlg::~PathSettingsDlg()
{

}

void PathSettingsDlg::accept()
{
	DataManager* pDataManager = DataManager::Instance();

	osg::Vec3d circleCenter;
	double dRadius;
	double dCircleTime;
	
	osg::Vec3d startPoint;
	osg::Vec3d endPoint;
	double dLineTime;

	circleCenter.x() = ui.lineEdit_CenterX->text().toDouble();
	circleCenter.y() = ui.lineEdit_CenterY->text().toDouble();
	circleCenter.z() = ui.lineEdit_Height->text().toDouble();
	dRadius = ui.lineEdit_Radius->text().toDouble();
	dCircleTime = ui.lineEdit_CircleTime->text().toDouble();

	startPoint.x() = ui.lineEdit_StartX->text().toDouble();
	startPoint.y() = ui.lineEdit_StartY->text().toDouble();
	startPoint.z() = ui.lineEdit_StartHeight->text().toDouble();
	endPoint.x() = ui.lineEdit_EndX->text().toDouble();
	endPoint.y() = ui.lineEdit_EndY->text().toDouble();
	endPoint.z() = ui.lineEdit_EndHeight->text().toDouble();
	dLineTime = ui.lineEdit_LineTime->text().toDouble();

	pDataManager->SetLinePathPara(startPoint, endPoint, dLineTime);
	pDataManager->SetCirclePathPara(circleCenter, dRadius, dCircleTime);

	QDialog::accept();
}

void PathSettingsDlg::reject()
{
	QDialog::reject();
}