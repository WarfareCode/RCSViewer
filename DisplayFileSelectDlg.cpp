#include "DisplayFileSelectDlg.h"

DisplayFileSelectDlg::DisplayFileSelectDlg(const QStringList& listFileTypeName, const QStringList& listFilePath, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	m_listFileTypeName = listFileTypeName;
	m_listFilePath = listFilePath;

	ui.comboBox_Video->addItem(QString::fromLocal8Bit("��"));

	int nCount = m_listFilePath.size();
	for (int i = 0; i < nCount; i++)
	{
		ui.comboBox_PlaneGPS->addItem(m_listFileTypeName[i] + " : " + m_listFilePath[i]);
		ui.comboBox_TargetGPS->addItem(m_listFileTypeName[i] + " : " + m_listFilePath[i]);
		ui.comboBox_RCS->addItem(m_listFileTypeName[i] + " : " + m_listFilePath[i]);
		ui.comboBox_Video->addItem(m_listFileTypeName[i] + " : " + m_listFilePath[i]);
	}

	int nCurrentIndex = 0;
	ui.comboBox_PlaneGPS->setCurrentIndex(nCurrentIndex++);

	if (nCurrentIndex >= nCount)
		nCurrentIndex = nCount - 1;
	ui.comboBox_TargetGPS->setCurrentIndex(nCurrentIndex++);

	if (nCurrentIndex >= nCount)
		nCurrentIndex = nCount - 1;
	ui.comboBox_RCS->setCurrentIndex(nCurrentIndex++);

	ui.comboBox_Video->setCurrentIndex(0);
}

DisplayFileSelectDlg::~DisplayFileSelectDlg()
{

}

void DisplayFileSelectDlg::getFilePath(QString& strPathPlaneGPS, QString& strPathTargetGPS, QString& strPathRCS, QString& strPathVideo)
{
	int nIndexPlaneGPS = ui.comboBox_PlaneGPS->currentIndex();
	int nIndexTargetGPS = ui.comboBox_TargetGPS->currentIndex();
	int nIndexRCS = ui.comboBox_RCS->currentIndex();
	int nIndexVideo = ui.comboBox_Video->currentIndex();

	if (nIndexVideo == 0)
	{
		strPathVideo = ""; //��Ƶ�ļ���һ���ǿ�
	}
	else
	{
		nIndexVideo--;
		strPathVideo = m_listFilePath[nIndexVideo];
	}

	strPathPlaneGPS = m_listFilePath[nIndexPlaneGPS];
	strPathTargetGPS = m_listFilePath[nIndexTargetGPS];
	strPathRCS = m_listFilePath[nIndexRCS];
}

