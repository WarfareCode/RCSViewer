#ifndef DATATYPE_H
#define DATATYPE_H

#include <QObject>
#include <QVector>

#define Class_Anno "ClassName"
#define Class_Fields "Fields"
#define Class_Guid "GUID"

#define Field_Name "FieldName"
#define Field_DataType "DataType"

enum FieldType
{
	FieldType_Int = 0,
	FieldType_Double = 1,
	FieldType_Bool = 2,
	FieldType_String = 3,
	FieldType_File = 4,
	FieldType_DateTime = 5,
	FieldType_OBMode = 6, //枚举类型
	FieldType_OBTarget = 7, //枚举类型
	FieldType_Polarity = 8,
	MaxCount = 9
};

extern QString g_strOBMode;
extern QString g_strOBTarget;
extern QString g_strPolarity;

#define FieldType_Int_Des "Int"
#define FieldType_Double_Des "Double"
#define FieldType_Bool_Des "Bool"
#define FieldType_String_Des "String"
#define FieldType_File_Des "File"
#define FieldType_DateTime_Des "DateTime"
#define FieldType_OBMode_Des g_strOBMode
#define FieldType_OBTarget_Des g_strOBTarget
#define FieldType_Polarity_Des g_strPolarity

class DataType : public QObject
{
	Q_OBJECT

public:

	static int typeCount();

	static QString typeDescription(int nIndex);

	static QString enumTypeDescription(const QString& enumName, int nIndex);

	static int enumTypeCount(const QString& enumName);

	DataType(QObject *parent);
	~DataType();

protected:

	static void initEnumType();

private:

	static QVector<QString> m_vecFieldDescription;

	static QVector<QString> m_vecOBModeDescription;

	static QVector<QString> m_vecOBTargetDescription;

	static QVector<QString> m_vecgPolarityDescription;
	
};

#endif // DATATYPE_H
