#include "SearchGlobalDlg.h"
#include <QFileInfo>
#include "Document.h"
#include <QSqlDatabase>
#include "MySqlTableModel.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTableView>
#include <QSqlField>
#include <QMessageBox>
#include <QDir>
#include "DataType.h"
#include "DisplayFileSelectDlg.h"
#include "DataManager.h"

extern bool classFile2JsonObj(QString filePath, QJsonObject& object);
void Show3DMainWindow();

bool FindFile(const QString &path, QStringList& listClassFile)
{
	QDir dir(path);
	if (!dir.exists())
	{
		return false;
	}
	dir.setFilter(QDir::Dirs | QDir::Files);
	dir.setSorting(QDir::DirsFirst);//文件夹排在前面
	QFileInfoList list = dir.entryInfoList();
	int i = 0;

	bool bIsDir;
	do
	{
		QFileInfo fileInfo = list.at(i);
		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
		{
			++i;
			continue;
		}
		bIsDir = fileInfo.isDir();
		if (bIsDir)
		{
			FindFile(fileInfo.filePath(), listClassFile);
		}
		else if (fileInfo.suffix().compare("class", Qt::CaseInsensitive) == 0)
		{
			listClassFile.push_back(fileInfo.absoluteFilePath());
		}
		++i;

	} while (i < list.size());

	return true;
}

void refreshClassInfo(QStringList listRecentProject, QMap<QString, QJsonObject>&mapAllClassInfo)
{
	QStringList listClassFile;
	for (auto str : listRecentProject)
	{
		QFileInfo fileInfo(str);
		QString strPath = fileInfo.path() + "/type";

		FindFile(strPath, listClassFile);
	}

	for (auto strFilePath : listClassFile)
	{
		QJsonObject object;
		if (classFile2JsonObj(strFilePath, object))
		{
			QString strTableName = object.value(Class_Guid).toString();
			mapAllClassInfo[strTableName] = object;
		}
	}
}

SearchGlobalDlg::SearchGlobalDlg(QStringList listRecentProject, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	setWindowTitle(QString::fromLocal8Bit("全局搜索"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	m_listRecentProject = listRecentProject;

	refreshClassInfo(listRecentProject, m_mapAllClassInfo);

	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(slotSearchGlobal())); 
	connect(ui.pushButton_Show3D, SIGNAL(clicked()), this, SLOT(slotShow3D()));
}

SearchGlobalDlg::~SearchGlobalDlg()
{

}

void SearchGlobalDlg::slotShow3D()
{
	QList<QTableWidgetItem *> listItem = ui.tableWidget->selectedItems();

	if (listItem.isEmpty())
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle(QString::fromLocal8Bit("提示"));
		msgBox.setText(QString::fromLocal8Bit("请选择需要展示的记录"));
		msgBox.exec();
		return;
	}

	int nRow = ui.tableWidget->row(listItem[0]);
	QString strCurrentTableName = m_listTableName[nRow];

	QJsonObject currentObject;
	QMap<QString, QJsonObject>::iterator itr = m_mapAllClassInfo.find(strCurrentTableName);
	if (itr != m_mapAllClassInfo.end())
	{
		currentObject = *itr;
	}
	
	//找到所有的文件字段
	QList<int> listFileIndex;
	QStringList listFileFieldName;
	QStringList listFilePath;
	QJsonArray fields = currentObject.value(Class_Fields).toArray();
	int nFieldCount = fields.count();
	for (int i = 0; i < nFieldCount; i++)
	{
		QJsonObject field = fields.at(i).toObject();
		QString strFieldType = field.value(Field_DataType).toString();
		QString strFieldName = field.value(Field_Name).toString();

		if (strFieldType.compare(FieldType_File_Des) == 0)
		{
			//0和1为工程和类型名，2为ID
			listFileIndex.push_back(i + 3);
			listFileFieldName.push_back(strFieldName);
		}
	}

	for (int i = 0; i < listFileIndex.size(); i++)
	{
		QTableWidgetItem* pItem = ui.tableWidget->item(nRow, listFileIndex[i]);
		listFilePath.push_back(pItem->text());
	}

	if (listFileFieldName.size() <= 0 || listFilePath.size() <= 0)
		return;

	DisplayFileSelectDlg dlg(listFileFieldName, listFilePath);
	if (dlg.exec())
	{
		accept();

		Show3DMainWindow();

		QString strPlaneGPS, strTargetGPS, strRCS, strVideo;
		dlg.getFilePath(strPlaneGPS, strTargetGPS, strRCS, strVideo);
		//DataManager::Instance()->LoadDataAndDisplay("d:/c/airbornegps.gps", "d:/c/targetgps.dat", "d:/c/targetrcs.rcs");
		DataManager::Instance()->LoadDataAndDisplay(strPlaneGPS, strTargetGPS, strRCS, strVideo);
	}
}

void SearchGlobalDlg::slotSearchGlobal()
{
	if (ui.lineEdit->text().isEmpty())
		return;

	m_listTableName.clear();
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
					//数据库中有记录，但是class已经被删除的，跳过。
					QMap<QString, QJsonObject>::iterator itr = m_mapAllClassInfo.find(strTable);
					if (itr == m_mapAllClassInfo.end())
						continue;

					listColumn.push_back(qMakePair(QString::fromLocal8Bit("工程"), str));
					//类型需要再加一个表来记录。后期加
					//listColumn.push_back(qMakePair(QString::fromLocal8Bit("类型"), ))

					QJsonObject& object = *itr;
					QString strClassName = object.value(Class_Anno).toString();
					listColumn.push_back(qMakePair(QString::fromLocal8Bit("类型"), strClassName + ".class"));

					int nCount = listFieldName.size();
					if (nMaxFieldCount < nCount)
					{
						nMaxFieldCount = nCount;
					}

					for (int i = 0; i < nCount; i ++)
					{
						listColumn.push_back(qMakePair(listFieldName[i], query.value(i).toString()));
					}

					if (!listColumn.empty())
					{
						records.push_back(listColumn);
						m_listTableName.push_back(strTable);
					}

					listColumn.clear();
				}
			}
		}
	}

	nMaxFieldCount += 2;
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