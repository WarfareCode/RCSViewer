#include "DataType.h"

QVector<QString> DataType::m_vecFieldDescription;

QVector<QString> DataType::m_vecOBModeDescription;
QVector<QString> DataType::m_vecOBTargetDescription;
QVector<QString> DataType::m_vecgPolarityDescription;

QString g_strOBMode = QString::fromLocal8Bit("观测模式");
QString g_strOBTarget = QString::fromLocal8Bit("观测目标");
QString g_strPolarity = QString::fromLocal8Bit("极化");

DataType::DataType(QObject *parent)
	: QObject(parent)
{

}

DataType::~DataType()
{

}

void DataType::initEnumType()
{
	if (m_vecOBModeDescription.isEmpty())
	{
		m_vecOBModeDescription.push_back(QString::fromLocal8Bit("过顶"));
		m_vecOBModeDescription.push_back(QString::fromLocal8Bit("固定俯仰角"));
	}

	if (m_vecOBTargetDescription.isEmpty())
	{
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("普通舰船"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("试验舰"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("靶标"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("战车"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("地杂波"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("海杂波"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("护卫舰"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("航母"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("驱逐舰"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("靶船"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("潜艇"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("坦克"));
		m_vecOBTargetDescription.push_back(QString::fromLocal8Bit("运输车"));
	}

	if (m_vecgPolarityDescription.isEmpty())
	{
		m_vecgPolarityDescription.push_back("HH");
		m_vecgPolarityDescription.push_back("HV");
		m_vecgPolarityDescription.push_back("VH");
		m_vecgPolarityDescription.push_back("VV");
	}
}

int DataType::typeCount()
{
	return MaxCount;
}

QString DataType::typeDescription(int nIndex)
{
	if (m_vecFieldDescription.isEmpty())
	{
		m_vecFieldDescription.push_back(FieldType_Int_Des);
		m_vecFieldDescription.push_back(FieldType_Double_Des);
		m_vecFieldDescription.push_back(FieldType_Bool_Des);
		m_vecFieldDescription.push_back(FieldType_String_Des);
		m_vecFieldDescription.push_back(FieldType_File_Des);
		m_vecFieldDescription.push_back(FieldType_DateTime_Des);
		m_vecFieldDescription.push_back(FieldType_OBMode_Des);
		m_vecFieldDescription.push_back(FieldType_OBTarget_Des);
		m_vecFieldDescription.push_back(FieldType_Polarity_Des);
	}

	if (nIndex < 0 || nIndex >= MaxCount)
	{
		return "";
	}
	else
	{
		return m_vecFieldDescription[nIndex];
	}
}

int DataType::enumTypeCount(const QString& enumName)
{
	initEnumType();

	if (enumName.compare(FieldType_OBMode_Des, Qt::CaseInsensitive) == 0)
	{
		return m_vecOBModeDescription.size();
	}
	else if (enumName.compare(FieldType_OBTarget_Des, Qt::CaseInsensitive) == 0)
	{
		return m_vecOBTargetDescription.size();
	}
	else if (enumName.compare(FieldType_Polarity_Des, Qt::CaseInsensitive) == 0)
	{
		return m_vecgPolarityDescription.size();
	}

	return 0;
}

QString DataType::enumTypeDescription(const QString& enumName, int nIndex)
{
	initEnumType();

	if (enumName.compare(FieldType_OBMode_Des, Qt::CaseInsensitive) == 0)
	{
		if (nIndex < 0 || nIndex >= m_vecOBModeDescription.size())
			return "";

		return m_vecOBModeDescription[nIndex];
	}
	else if (enumName.compare(FieldType_OBTarget_Des, Qt::CaseInsensitive) == 0)
	{
		if (nIndex < 0 || nIndex >= m_vecOBTargetDescription.size())
			return "";

		return m_vecOBTargetDescription[nIndex];
	}
	else if (enumName.compare(FieldType_Polarity_Des, Qt::CaseInsensitive) == 0)
	{
		if (nIndex < 0 || nIndex >= m_vecgPolarityDescription.size())
			return "";

		return m_vecgPolarityDescription[nIndex];
	}

	return "";
}