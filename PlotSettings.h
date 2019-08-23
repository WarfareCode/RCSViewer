#pragma once

#include <QString>

class PlotSettings
{

public:

	static PlotSettings* Instance()
	{
		static PlotSettings settings;
		return &settings;
	}

	QString hLabel();
	void setHLabel(QString strLabel);

	QString vLabel();
	void setVLabel(QString strLabel);

	void getHMinMax(int& nMin, int& nMax);
	void setHMinMax(int nMin, int nMax);

	void getVMinMax(int& nMin, int& nMax);
	void setVMinMax(int nMin, int nMax);

private:

	QString m_strHLabel;

	QString m_strVLabel;

	int m_nHMin;

	int m_nHMax;

	int m_nVMin;

	int m_nVMax;

	QString m_strPath;

protected:

	PlotSettings();
	~PlotSettings();

	PlotSettings(const PlotSettings&);
	PlotSettings& operator= (const PlotSettings&);
};

