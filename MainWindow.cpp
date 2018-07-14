#include "MainWindow.h"
#include <osgViewer/Viewer>
#include "ViewerWidget.h"
#include <QtWidgets/QFileDialog>
#include "DataManager.h"
#include "AeroRotateDlg.h"
#include <QtWidgets/QActionGroup>
#include "PathSettingsDlg.h"
#include "TargetSettingsDlg.h"
#include <QtWidgets/QDockWidget>
#include "RadarBeamRotateDlg.h"

extern osgViewer::View* g_pView;

void SetTerrainManipulator();
void SetNodeTrackerManipulator();

class MyWidget : public QWidget
{
public:
	QSize sizeHint() const
	{
		return QSize(400, 900); /* 在这里定义dock的初始大小 */
	}
};

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowTitle(QString::fromLocal8Bit("模拟"));

	QMenu* pMenuFile = menuBar()->addMenu(QString::fromLocal8Bit("文件"));
	QAction* pActionLoadAeroplane = pMenuFile->addAction(QString::fromLocal8Bit("加载飞行器"));
	QAction* pActionLoadObservedObj = pMenuFile->addAction(QString::fromLocal8Bit("加载观测目标"));

	connect(pActionLoadAeroplane, SIGNAL(triggered()), this, SLOT(slotLoadAeroplane()));
	connect(pActionLoadObservedObj, SIGNAL(triggered()), this, SLOT(slotLoadObservedObj()));

	QMenu* pMenuManipulator = menuBar()->addMenu(QString::fromLocal8Bit("视角模式"));
	QAction* pActionAero = pMenuManipulator->addAction(QString::fromLocal8Bit("飞行器"));
	QAction* pActionTerrain = pMenuManipulator->addAction(QString::fromLocal8Bit("地面"));

	pActionAero->setCheckable(true);
	pActionAero->setChecked(true);
	pActionTerrain->setCheckable(true);

	QActionGroup* pActonGroup = new QActionGroup(pMenuManipulator);
	pActonGroup->addAction(pActionAero);
	pActonGroup->addAction(pActionTerrain);

	connect(pActionAero, SIGNAL(triggered()), this, SLOT(slotSetManipulatorAeroplane()));
	connect(pActionTerrain, SIGNAL(triggered()), this, SLOT(slotSetManipulatorTerrain()));

	QMenu* pMenuSettings = menuBar()->addMenu(QString::fromLocal8Bit("飞行器姿态"));
	QAction* pActionAeroplanePos = pMenuSettings->addAction(QString::fromLocal8Bit("参数设置"));

	connect(pActionAeroplanePos, SIGNAL(triggered()), this, SLOT(slotSetAeroplaneMatrix()));

	QMenu* pMenuAeroplanePath = menuBar()->addMenu(QString::fromLocal8Bit("飞行轨迹"));
	QAction* pCirclePath = pMenuAeroplanePath->addAction(QString::fromLocal8Bit("环形"));
	QAction* pLinePath = pMenuAeroplanePath->addAction(QString::fromLocal8Bit("直线"));
	pMenuAeroplanePath->addSeparator();
	QAction* pActionPathPara = pMenuAeroplanePath->addAction(QString::fromLocal8Bit("参数设置"));

	pCirclePath->setCheckable(true);
	pCirclePath->setChecked(true);
	pLinePath->setCheckable(true);

	QActionGroup* pActonGroup2 = new QActionGroup(pMenuAeroplanePath);
	pActonGroup2->addAction(pCirclePath);
	pActonGroup2->addAction(pLinePath);

	connect(pCirclePath, SIGNAL(triggered()), this, SLOT(slotCirclePath()));
	connect(pLinePath, SIGNAL(triggered()), this, SLOT(slotLinePath()));
	connect(pActionPathPara, SIGNAL(triggered()), this, SLOT(slotPathPara()));

	QMenu* pMenuTargetSettings = menuBar()->addMenu(QString::fromLocal8Bit("目标参数"));
	QAction* pActionTargetSettings = pMenuTargetSettings->addAction(QString::fromLocal8Bit("设置"));

	connect(pActionTargetSettings, SIGNAL(triggered()), this, SLOT(slotTargetPara()));

	QMenu* pMenuRadarBeamSettings = menuBar()->addMenu(QString::fromLocal8Bit("波束方向"));
	QAction* pActionRadarBeamSettings = pMenuRadarBeamSettings->addAction(QString::fromLocal8Bit("设置"));

	connect(pActionRadarBeamSettings, SIGNAL(triggered()), this, SLOT(slotRadarBeamPara()));

#if QT_VERSION >= 0x050000
	// Qt5 is currently crashing and reporting "Cannot make QOpenGLContext current in a different thread" when the viewer is run multi-threaded, this is regression from Qt4
	osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::SingleThreaded;
#else
	osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::CullDrawThreadPerContext;
#endif

// 	QDockWidget* pDockWidget = new QDockWidget;
// 	pDockWidget->setWidget(new MyWidget);
// 	addDockWidget(Qt::DockWidgetArea::NoDockWidgetArea, pDockWidget);
// 
// 	QDockWidget* pDockWidget2 = new QDockWidget;
// 	pDockWidget2->setWidget(new MyWidget);
// 	addDockWidget(Qt::DockWidgetArea::NoDockWidgetArea, pDockWidget2);

	ViewerWidget* viewWidget = new ViewerWidget(threadingModel);
	setCentralWidget(viewWidget);
}

MainWindow::~MainWindow()
{

}

void MainWindow::slotSetManipulatorAeroplane()
{
	QAction* pAction = qobject_cast<QAction*>(sender());
	pAction->setChecked(true);

	SetNodeTrackerManipulator();
}

void MainWindow::slotSetManipulatorTerrain()
{
	QAction* pAction = qobject_cast<QAction*>(sender());
	pAction->setChecked(true);

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
	QString strFileName = QFileDialog::getOpenFileName();
	if (strFileName.isEmpty())
		return;

	DataManager::Instance()->LoadTargetObject(strFileName);
}

void MainWindow::slotSetAeroplaneMatrix()
{
	AeroRotateDlg dlg;
	dlg.exec();
}

void MainWindow::slotCirclePath()
{
	QAction* pAction = qobject_cast<QAction*>(sender());
	pAction->setChecked(true);

	DataManager::Instance()->SetPathType(DataManager::PathType_Circle);
}

void MainWindow::slotLinePath()
{
	QAction* pAction = qobject_cast<QAction*>(sender());
	pAction->setChecked(true);

	DataManager::Instance()->SetPathType(DataManager::PathType_Line);
}

void MainWindow::slotPathPara()
{
	PathSettingsDlg dlg;
	dlg.exec();
}

void MainWindow::slotTargetPara()
{
	TargetSettingsDlg dlg;
	dlg.exec();
}

void MainWindow::slotRadarBeamPara()
{
	RadarBeamRotateDlg dlg;
	dlg.exec();
}