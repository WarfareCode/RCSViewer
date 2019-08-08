#include "SearchGlobalDlg.h"
#include <QFileInfo>
#include "Document.h"
#include <QSqlDatabase>
#include "MySqlTableModel.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTableView>
#include <QSqlField>

SearchGlobalDlg::SearchGlobalDlg(QStringList listRecentProject, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	setWindowTitle(QString::fromLocal8Bit("全局搜索"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	m_listRecentProject = listRecentProject;
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(slotSearchGlobal()));
}

SearchGlobalDlg::~SearchGlobalDlg()
{

}

void SearchGlobalDlg::slotSearchGlobal()
{
	ui.tableWidget->clear();
	QString strKey = ui.lineEdit->text();

	if (strKey.isEmpty())
		return;

	int nMaxFieldCount = 0;
	QList<QList<QPair<QString, QString>>> records;

	for (auto str : m_listRecentProject)
	{
		QFileInfo fileInfo(str);
		QString strDBFile = fileInfo.path() + "/" + DBFILENAME;

		QSqlDatabase database = QSqlDatabase::database();
		if (!database.isValid())
		{
			database = QSqlDatabase::addDatabase("QSQLITE");
		}

		database.close();
		database.setDatabaseName(strDBFile);
		if(!database.open())
			continue;

		QStringList tables = database.tables();

		for (auto strTable : tables)
		{
			if (strTable[0] != 't')
				continue;

			QString strq = "PRAGMA table_info(" + strTable + ")";
			QSqlQuery q(strq);

			QStringList listFieldName;

			while (q.next())
			{
				QString country = q.value(1).toString();
				listFieldName.push_back(country);
			}

			for (auto strFieldName : listFieldName)
			{
				QString strQuery = "SELECT * FROM ";
				strQuery += strTable;
				strQuery += " WHERE ";
				strQuery += strFieldName;
				strQuery += " LIKE ";
				strQuery += "'%";
				strQuery += strKey;
				strQuery += "%'";

				QSqlQuery query(strQuery);

				QList<QPair<QString, QString>> listColumn;

				while (query.next())
				{
					if (listColumn.empty())
					{
						listColumn.push_back(qMakePair(QString::fromLocal8Bit("工程"), str));
						//类型需要再加一个表来记录。后期加
						//listColumn.push_back(qMakePair(QString::fromLocal8Bit("类型"), ))
					}

					int nCount = listFieldName.size();
					if (nMaxFieldCount < nCount)
					{
						nMaxFieldCount = nCount;
					}

					for (int i = 0; i < nCount; i ++)
					{
						listColumn.push_back(qMakePair(listFieldName[i], query.value(i).toString()));
					}
				}

				if (!listColumn.empty())
				{
					records.push_back(listColumn);
				}
			}
		}
	}

	nMaxFieldCount += 1;
	ui.tableWidget->setColumnCount(nMaxFieldCount);
	ui.tableWidget->setRowCount(records.size());

	for (int j = 0; j < records.size(); j++)
	{
		QList<QPair<QString, QString>>& listColumn = records[j];
		for (int i = 0; i < listColumn.size(); i ++)
		{
			QTableWidgetItem* pItem = new QTableWidgetItem(listColumn[i].second);
			pItem->setToolTip(listColumn[i].second);
			ui.tableWidget->setItem(j, i, pItem);
		}
	}
}