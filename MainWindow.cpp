#include "MainWindow.h"
#include <osgViewer/Viewer>
#include "ViewerWidget.h"
#include <QtWidgets/QFileDialog>
#include "DataManager.h"

extern osgViewer::View* g_pView;

void SetTerrainManipulator();
void SetNodeTrackerManipulator();

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowTitle(QString::fromLocal8Bit("模拟"));

	QMenu* pMenuFile = menuBar()->addMenu(QString::fromLocal8Bit("文件"));
	QAction* pActionLoadAeroplane = pMenuFile->addAction(QString::fromLocal8Bit("加载飞行器"));
	QAction* pActionLoadObservedObj = pMenuFile->addAction(QString::fromLocal8Bit("加载观测目标"));

	connect(pActionLoadAeroplane, SIGNAL(triggered()), this, SLOT(slotLoadAeroplane()));
	connect(pActionLoadObservedObj, SIGNAL(triggered()), this, SLOT(slotLoadObservedObj()));

	QMenu* pMenuManipulator = menuBar()->addMenu(QString::fromLocal8Bit("视角"));
	QAction* pActionAero = pMenuManipulator->addAction(QString::fromLocal8Bit("飞行器"));
	QAction* pActionTerrain = pMenuManipulator->addAction(QString::fromLocal8Bit("地面"));

	connect(pActionAero, SIGNAL(triggered()), this, SLOT(slotSetManipulatorAeroplane()));
	connect(pActionTerrain, SIGNAL(triggered()), this, SLOT(slotSetManipulatorTerrain()));

	QMenu* pMenuSettings = menuBar()->addMenu(QString::fromLocal8Bit("设置"));
	QAction* pActionAeroplanePos = pMenuSettings->addAction(QString::fromLocal8Bit("飞行器姿态"));

	connect(pActionAeroplanePos, SIGNAL(triggered()), this, SLOT(slotSetAeroplaneMatrix()));

#if QT_VERSION >= 0x050000
	// Qt5 is currently crashing and reporting "Cannot make QOpenGLContext current in a different thread" when the viewer is run multi-threaded, this is regression from Qt4
	osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::SingleThreaded;
#else
	osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::CullDrawThreadPerContext;
#endif

	ViewerWidget* viewWidget = new ViewerWidget(threadingModel);
	setCentralWidget(viewWidget);
}

MainWindow::~MainWindow()
{

}

void MainWindow::slotSetManipulatorAeroplane()
{
	SetNodeTrackerManipulator();
}

void MainWindow::slotSetManipulatorTerrain()
{
	SetTerrainManipulator();
}

void MainWindow::slotLoadAeroplane()
{
	QString strFileName = QFileDialog::getOpenFileName();
	if (strFileName.isEmpty())
		return;

	DataManager::Instance()->LoadAerocraft(strFileName);

	QString strClassName = g_pView->getCameraManipulator()->className();
	if (strClassName.compare("NodeTrackerManipulator") == 0)
	{
		SetNodeTrackerManipulator();
	}
}

void MainWindow::slotLoadObservedObj()
{

}

void MainWindow::slotSetAeroplaneMatrix()
{

}