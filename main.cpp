#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <windows.h>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>

int main(int argc, char *argv[])
{
	{
		TCHAR szFilePath[MAX_PATH + 1] = { 0 };
		GetModuleFileName(NULL, szFilePath, MAX_PATH);

		std::string strPath = osgDB::getFilePath(szFilePath);
		strPath += "\\resources";
		osgDB::Registry::instance()->getDataFilePathList().push_back(strPath);
	}
	QApplication a(argc, argv);

	MainWindow w;
	w.showMaximized();
	return a.exec();
}
