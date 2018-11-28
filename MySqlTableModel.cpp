#include "MySqlTableModel.h"
#include "DataType.h"
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

MySqlTableModel::MySqlTableModel(QObject *parent)
	: QSqlTableModel(parent)
{

}

MySqlTableModel::~MySqlTableModel()
{

}

void MySqlTableModel::SetJsonObj(const QJsonObject& currentObj)
{
	m_mapFileIndex.clear();
	m_currentObj = currentObj;

	QJsonArray fields = m_currentObj.value(Class_Fields).toArray();
	for (int i = 0; i < fields.count(); i++)
	{
		QJsonObject field = fields.at(i).toObject();
		QString dataType = field.value(Field_DataType).toString();
		QString fieldName = field.value(Field_Name).toString();

		if (dataType == FieldType_File_Des)
		{
			m_mapFileIndex.insert(i + 1, i + 1);
		}
	}
}

Qt::ItemFlags MySqlTableModel::flags(const QModelIndex &index) const
{
	int nCol = index.column();
	QMap<int, int>::const_iterator itr = m_mapFileIndex.find(nCol);
	if (itr != m_mapFileIndex.end())
	{
		return QSqlQueryModel::flags(index);
	}
	else
	{
		return QSqlTableModel::flags(index);
	}
}