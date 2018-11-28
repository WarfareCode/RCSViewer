#ifndef MYSQLTABLEMODEL_H
#define MYSQLTABLEMODEL_H

#include <QSqlTableModel>
#include <QtCore/QJsonObject>

//派生一个model。用于控制“文件”字段不能被编辑。
class MySqlTableModel : public QSqlTableModel
{
	Q_OBJECT

public:
	MySqlTableModel(QObject *parent);
	~MySqlTableModel();

	Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

	void SetJsonObj(const QJsonObject& currentObj);

private:

	QJsonObject m_currentObj;
	QMap<int, int> m_mapFileIndex;
};

#endif // MYSQLTABLEMODEL_H
