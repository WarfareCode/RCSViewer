#include "MainWindow3D.h"
#include <QtWidgets/QApplication>
#include <windows.h>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include "Mainwindow.h"
#include "osg/Matrixf"
#include "samplingthread.h"

int main(int argc, char *argv[])
{
	{
		TCHAR szFilePath[MAX_PATH + 1] = { 0 };
		GetModuleFileName(NULL, szFilePath, MAX_PATH);

		std::string strPath = osgDB::getFilePath(szFilePath);
		strPath += "\\resources";
		osgDB::Registry::instance()->getDataFilePathList().push_back(strPath);
	}

//	QApplication a(argc, argv);
// 	MainWindow3D w;
// 	w.showMaximized();
// 	return a.exec();

	QApplication app(argc, argv);
	app.setStyle(new RibbonStyle);

	app.setApplicationName("QtitanRibbon Controls Sample");
	app.setOrganizationName("Developer Machines");

	MainWindow mainWindow;
	mainWindow.loadRencentProjectConfig();
	//mainWindow.resize(1000, 600);
	mainWindow.showMaximized();

// 	SamplingThread samplingThread;
// 	samplingThread.setFrequency(0.05);
// 	samplingThread.setAmplitude(160.0);
// 	samplingThread.setInterval(10.0);
// 	g_pSampleThread = &samplingThread;

	int nRes = app.exec();
	mainWindow.saveRencentProjectConfig();

// 	samplingThread.stop();
// 	samplingThread.wait(1000);

	return nRes;
}
