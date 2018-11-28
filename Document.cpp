#include "Document.h"
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtDebug>
#include <QSqlError>
#include <QSqlRecord>
#include "DataType.h"
#include <QtWidgets/QMessageBox>

QString Document::m_strProjectDir;
bool Document::m_bValid = false;
bool Document::s_bCopyFile = false;
QStringList Document::m_listRecentProject;

extern bool classFile2JsonObj(QString filePath, QJsonObject& object);

Document::Document()
{

}

Document::~Document()
{

}

bool Document::needCopyFile()
{
	return s_bCopyFile;
}

QString Document::projectDir()
{
	return m_strProjectDir;
}

QString Document::typeDir()
{
	return m_strProjectDir + "/" + TYPEDIR;
}

QString Document::dataDir()
{
	return m_strProjectDir + "/" + DATADIR;
}

QString Document::sqliteFilePath()
{
	return m_strProjectDir + "/" + DBFILENAME;
	//return DBFILENAME;
}

bool Document::isProjectValid()
{
	return m_bValid;
}

bool Document::openProject(const QString& strDir)
{
	m_strProjectDir = strDir;

	QSqlDatabase database;
// 	if (QSqlDatabase::contains("qt_sql_default_connection"))
// 	{
// 		database = QSqlDatabase::database("qt_sql_default_connection");
// 		database.close();
// 
// 		QSqlDatabase::removeDatabase("qt_sql_default_connection");
// 	}

	database = QSqlDatabase::addDatabase("QSQLITE");
	database.setDatabaseName(sqliteFilePath());
	// 	database.setUserName("XingYeZhiXia");
	// 	database.setPassword("123456");

	if (!database.open())
	{
		m_bValid = false;
		return false;
	}

	//database.close();

	QDir dirData(dataDir());
	if (!dirData.exists())
	{
		m_bValid = false;
		return false;
	}

	QDir dirType(typeDir());
	if (!dirType.exists())
	{
		m_bValid = false;
		return false;
	}

	m_bValid = true;
	return true;
}

bool Document::newProject(const QString& strDir)
{
	m_strProjectDir = strDir;

	QDir dirAll(strDir);
	dirAll.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	QFileInfoList strList = dirAll.entryInfoList();

	//空文件夹才允许新建工程。防止误删数据
	if (!strList.isEmpty())
	{
		m_bValid = false;
		return false;
	}

	QDir dirData(dataDir());
	if (!dirData.exists())
	{
		dirData.mkdir(dataDir());
	}

	QDir dirType(typeDir());
	if (!dirType.exists())
	{
		dirType.mkdir(typeDir());
	}

	QSqlDatabase database;
	database = QSqlDatabase::addDatabase("QSQLITE");
	database.setDatabaseName(sqliteFilePath());
// 	database.setUserName("XingYeZhiXia");
// 	database.setPassword("123456");

	if (!database.open())
	{
		qDebug() << "Error: Failed to connect database." << database.lastError();
		m_bValid = false;
		return false;
	}
	else
	{
		// do something
	}

	m_bValid = true;
	return true;
}

bool Document::addRecord(const QString& strClassFilePath, const QJsonObject& classType, const QJsonArray& record)
{
	if (!m_bValid)
		return false;

	QString strGUID = classType.value(Class_Guid).toString();
	//QString create_sql = "create table student (id int primary key, name varchar(30), age int)";
	QString create_sql = "CREATE TABLE IF NOT EXISTS " + strGUID + "(";
	QString insert_sql = "insert into " + strGUID + " VALUES(";

	create_sql += "ID INTEGER PRIMARY KEY AUTOINCREMENT,";
	insert_sql += "null,";

	QStringList listStrFileName;
		
	QJsonArray fields = classType.value(Class_Fields).toArray();
	for (int i = 0; i < fields.count(); i ++)
	{
		QJsonObject field = fields.at(i).toObject();
		QString dataType = field.value(Field_DataType).toString();
		QString fieldName = field.value(Field_Name).toString();

		QJsonValue value = record.at(i);

		create_sql += fieldName;
		create_sql += " ";

		if (dataType == FieldType_Int_Des)
		{
			create_sql += "integer";
			insert_sql += QString::number(value.toInt());
		}
		else if (dataType == FieldType_Double_Des)
		{
			create_sql += "real";
			insert_sql += QString::number(value.toDouble());
		}
		else if (dataType == FieldType_Bool_Des)
		{
			create_sql += "boolean";
			insert_sql += QString::number(value.toInt());
		}
		else if (dataType == FieldType_String_Des)
		{
			create_sql += "text";
			insert_sql += "'";
			insert_sql += value.toString();
			insert_sql += "'";
		}
		else if (dataType == FieldType_File_Des)
		{
			QFileInfo fileInfo(value.toString());
			create_sql += "text";
			insert_sql += "'";
			
			if (s_bCopyFile)
			{
				insert_sql += fileInfo.fileName();
			}
			else
			{
				insert_sql += value.toString();
			}

			insert_sql += "'";

			listStrFileName.push_back(value.toString());
		}
		else if (dataType == FieldType_DateTime_Des)
		{
			create_sql += "datetime";
			insert_sql += "'";
			insert_sql += value.toString();
			insert_sql += "'";
		}
		else if (dataType == FieldType_OBMode_Des 
			|| dataType == FieldType_OBTarget_Des
			|| dataType == FieldType_Polarity_Des)
		{
			create_sql += "text";
			insert_sql += "'";
			insert_sql += value.toString();
			insert_sql += "'";
		}

		if (i != fields.count() - 1)
		{
			create_sql += ",";
			insert_sql += ",";
		}
				
	}

	create_sql += ");";
	insert_sql += ");";

	if (s_bCopyFile)
	{
		QString strDesDir = QFileInfo(strClassFilePath).absolutePath();
		strDesDir.remove(m_strProjectDir);
		strDesDir.remove(0, 5);
		strDesDir = "/data" + strDesDir;
		strDesDir = m_strProjectDir + strDesDir;

		//判断目标文件夹是否已经有同名文件
		QDir dirAll(strDesDir);
		dirAll.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
		QFileInfoList strList = dirAll.entryInfoList();

		bool bError = false;
		QString strSrcFileName;

		for (QFileInfoList::iterator itr = strList.begin(); itr != strList.end(); itr++)
		{
			QFileInfo& fileInfo = *itr;

			for (int i = 0; i < listStrFileName.size(); i++)
			{
				strSrcFileName = QFileInfo(listStrFileName[i]).fileName();
				if (fileInfo.fileName().compare(strSrcFileName, Qt::CaseInsensitive) == 0)
				{
					bError = true;
					break;
				}
			}
		}

		if (bError)
		{
			QMessageBox box;
			box.setWindowTitle(QString::fromLocal8Bit("提示"));
			box.setText(QString::fromLocal8Bit("包含同名文件或者该文件已经入库，请重命名：") + strSrcFileName);
			box.exec();

			//database.close();
			return false;
		}

		QDir dirData(strDesDir);
		if (!dirData.exists())
		{
			dirData.mkpath(strDesDir);
		}

		for (int i = 0; i < listStrFileName.size(); i++)
		{
			QString strSrcFile = listStrFileName[i];
			QString strDstFile = strDesDir + "/" + QFileInfo(strSrcFile).fileName();
			QFile::copy(strSrcFile, strDstFile);
		}
	}
		
	//创建表
	QSqlQuery sql_query;
	sql_query.prepare(create_sql);
	if (!sql_query.exec())
	{
		qDebug() << "Error: Fail to create table." << sql_query.lastError();
	}
	else
	{
		qDebug() << "Table created!";
	}

	//添加记录
	sql_query.prepare(insert_sql);
	if (!sql_query.exec())
	{
		qDebug() << "Error: Fail to insert table." << sql_query.lastError();
	}
	else
	{
		qDebug() << "insert success!";
	}

	return true;
}

bool Document::deleteClass(const QString& strFilePath)
{
	QJsonObject object;
	QString strGUID = object.value(Class_Guid).toString();

	if (Document::needCopyFile() && classFile2JsonObj(strFilePath, object))
	{
		QStringList listFileField;
		QJsonArray fields = object.value(Class_Fields).toArray();
		for (int i = 0; i < fields.count(); i++)
		{
			QJsonObject field = fields.at(i).toObject();
			QString dataType = field.value(Field_DataType).toString();
			QString fieldName = field.value(Field_Name).toString();

			if (dataType == FieldType_File_Des)
			{
				listFileField.push_back(fieldName);
			}
		}

		//数据库处理
		QFileInfo fileInfo(strFilePath);
		QString strSQL = "SELECT * FROM " + strGUID;
		QSqlQuery query(strSQL);

		while (query.next())
		{
			for (QStringList::iterator itr = listFileField.begin(); itr != listFileField.end(); itr++)
			{
				QString strCurrentDir = fileInfo.path();
				QString strFile = query.value(*itr).toString();

				QString strDataDir = Document::dataDir();
				QString strTypeDir = Document::typeDir();
				strCurrentDir.remove(0, strTypeDir.size());
				strCurrentDir = strDataDir + strCurrentDir + "/" + strFile;
				QFile::remove(strCurrentDir);
			}
		}
	}

	QSqlQuery queryDelete;
	queryDelete.exec("drop table " + strGUID);
	QFile::remove(strFilePath);

	return true;
}