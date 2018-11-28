#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtCore/QString>
#include <QJsonObject>
#include <QJsonArray>

#define DBFILENAME "data.db"
//#define DBNAME "Database"
#define TYPEDIR "type"
#define DATADIR "data"


class Document
{
public:
	Document();
	~Document();

public:

	static bool openProject(const QString& strDir);

	static bool newProject(const QString& strDir);

	static bool isProjectValid();

	static bool addRecord(const QString& strClassFilePath, const QJsonObject& classType, const QJsonArray& record);

	static QString typeDir();

	static QString dataDir();

	static QString projectDir();

	static QString sqliteFilePath();

	static bool deleteClass(const QString& strClassFilePath);

	static bool needCopyFile();

private:

	static bool s_bCopyFile;

	static QString m_strProjectDir;

	static bool m_bValid;

	//最近工程
	static QStringList m_listRecentProject;

};

#endif // DOCUMENT_H
