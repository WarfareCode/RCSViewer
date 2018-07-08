#include "AeroRotateDlg.h"
#include "DataManager.h"

AeroRotateDlg::AeroRotateDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	DataManager* pDataManager = DataManager::Instance();
	float x, y, z;
	pDataManager->GetAerocraftRotate(x, y, z);

	
}

AeroRotateDlg::~AeroRotateDlg()
{

}
