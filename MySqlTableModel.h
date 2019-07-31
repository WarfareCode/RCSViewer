#ifndef MYSQLTABLEMODEL_H
#define MYSQLTABLEMODEL_H

#include <QSqlTableModel>
#include <QtCore/QJsonObject>

#include <QtWidgets/QToolTip>

class MyModel : public QSqlTableModel
{
public:

	MyModel(QObject *parent, QSqlDatabase db/*QObject* parent*/) : QSqlTableModel(parent, db){}
	~MyModel(){}

	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
	{
		if (role == Qt::ToolTipRole) {
			QString name = index.data(Qt::DisplayRole).toString();
			QToolTip::showText(QCursor::pos(), name);
		}

		return QSqlTableModel::data(index, role);
	}
};

//����һ��model�����ڿ��ơ��ļ����ֶβ��ܱ��༭��
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
