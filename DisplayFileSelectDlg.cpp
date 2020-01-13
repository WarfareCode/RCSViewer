#include "DisplayFileSelectDlg.h"

DisplayFileSelectDlg::DisplayFileSelectDlg(const QStringList& listFileTypeName, const QStringList& listFilePath, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	setWindowTitle(QString::fromLocal8Bit("选择展示数据"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowIcon(QIcon());

	m_listFileTypeName = listFileTypeName;
	m_listFilePath = listFilePath;

	ui.comboBox_Video->addItem(QString::fromLocal8Bit("无"));

	int nCount = m_listFilePath.size();
	for (int i = 0; i < nCount; i++)
	{
		ui.comboBox_PlaneGPS->addItem(m_listFileTypeName[i] + " : " + m_listFilePath[i]);
		ui.comboBox_TargetGPS->addItem(m_listFileTypeName[i] + " : " + m_listFilePath[i]);
		ui.comboBox_RCS->addItem(m_listFileTypeName[i] + " : " + m_listFilePath[i]);
		ui.comboBox_Video->addItem(m_listFileTypeName[i] + " : " + m_listFilePath[i]);
	}

	int nPlaneGPSIndex = 0;
	int nTargetGPSIndex = 0;
	int nRCSIndex = 0;
	int nVideoIndex = 0;

	for (int i = 0; i < m_listFileTypeName.size(); i ++)
	{
		QString& strField = m_listFileTypeName[i];
		if (strField.contains("plane", Qt::CaseInsensitive) 
			|| strField.contains(QString::fromLocal8Bit("飞")))
		{
			nPlaneGPSIndex = i;
		}

		if (strField.contains("target", Qt::CaseInsensitive)
			|| strField.contains(QString::fromLocal8Bit("目标")))
		{
			nTargetGPSIndex = i;
		}

		if (strField.contains("rcs", Qt::CaseInsensitive)
			|| strField.contains("sar", Qt::CaseInsensitive))
		{
			nRCSIndex = i;
		}

		if (strField.contains("video", Qt::CaseInsensitive)
			|| strField.contains(QString::fromLocal8Bit("视频")))
		{
			nVideoIndex = i + 1;
		}
	}

	ui.comboBox_PlaneGPS->setCurrentIndex(nPlaneGPSIndex);
	ui.comboBox_TargetGPS->setCurrentIndex(nTargetGPSIndex);
	ui.comboBox_RCS->setCurrentIndex(nRCSIndex);
	ui.comboBox_Video->setCurrentIndex(nVideoIndex);
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
		strPathVideo = ""; //视频文件第一项是空
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

