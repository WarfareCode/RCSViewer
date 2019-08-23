#include "PlotSettings.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QSettings>

PlotSettings::PlotSettings()
{
	m_strPath = QCoreApplication::applicationDirPath() + "/plotSettings.ini";
	QFileInfo fileInfo(m_strPath);

	m_strHLabel = QString::fromLocal8Bit("¶È");
	m_strVLabel = QString::fromLocal8Bit("db");

	m_nHMin = 0;
	m_nHMax = 360;

	m_nVMin = 20;
	m_nVMax = 60;

	if (fileInfo.exists())
	{
		QSettings  iniFile(m_strPath, QSettings::IniFormat);
		m_strVLabel = iniFile.value("VLabel").toString();
		m_strHLabel = iniFile.value("HLabel").toString();

		m_nHMin = iniFile.value("HMin").toInt();
		m_nHMax = iniFile.value("HMax").toInt();

		m_nVMin = iniFile.value("VMin").toInt();
		m_nVMax = iniFile.value("VMax").toInt();
	}
}


PlotSettings::~PlotSettings()
{
	QSettings  iniFile(m_strPath, QSettings::IniFormat);

	iniFile.setValue("VLabel", m_strVLabel);
	iniFile.setValue("HLabel", m_strHLabel);

	iniFile.setValue("HMin", m_nHMin);
	iniFile.setValue("HMax", m_nHMax);

	iniFile.setValue("VMin", m_nVMin);
	iniFile.setValue("VMax", m_nVMax);
}

QString PlotSettings::hLabel()
{
	return m_strHLabel;
}

void PlotSettings::setHLabel(QString strLabel)
{
	m_strHLabel = strLabel;
}

QString PlotSettings::vLabel()
{
	return m_strVLabel;
}

void PlotSettings::setVLabel(QString strLabel)
{
	m_strVLabel = strLabel;
}

void PlotSettings::getHMinMax(int& nMin, int& nMax)
{
	nMin = m_nHMin;
	nMax = m_nHMax;
}

void PlotSettings::setHMinMax(int nMin, int nMax)
{
	m_nHMin = nMin;
	m_nHMax = nMax;
}

void PlotSettings::getVMinMax(int& nMin, int& nMax)
{
	nMin = m_nVMin;
	nMax = m_nVMax;
}

void PlotSettings::setVMinMax(int nMin, int nMax)
{
	m_nVMin = nMin;
	m_nVMax = nMax;
}
