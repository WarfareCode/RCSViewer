#include "MainWindow3D.h"
#include <osgViewer/Viewer>

#include "ViewerWidget.h"
#include <QtWidgets/QFileDialog>
#include "DataManager.h"
#include "AeroRotateDlg.h"
#include <QtWidgets/QActionGroup>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimediaWidgets/QVideoWidget>
#include "PathSettingsDlg.h"
#include "TargetSettingsDlg.h"
#include <QtWidgets/QDockWidget>
#include "RadarBeamRotateDlg.h"
#include "videoplayer.h"
#include <QtWidgets/QMenuBar>
#include "samplingthread.h"
#include "signaldata.h"
#include "SetPathDlg.h"
#include <QtWidgets/QMessageBox>
#include "PlotRangeDlg.h"
#include "PlotSettings.h"

extern osgViewer::View* g_pView;
extern SamplingThread* g_pSampleThread;
extern Plot* g_pPlot;
extern VideoPlayer* g_pVideoPlayer;

void SetTerrainManipulator();
void SetNodeTrackerManipulator(int nIndex = 0);
void SetAutoManipulator(double, double, double, double, double);

// class MyWidget : public QWidget
// {
// public:
// 	QSize sizeHint() const
// 	{
// 		return QSize(400, 900); /* 在这里定义dock的初始大小 */
// 	}
// };

MainWindow3D::MainWindow3D(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowTitle(QString::fromLocal8Bit("模拟"));

	m_pProcess = nullptr;

	QMenu* pMenuFile = menuBar()->addMenu(QString::fromLocal8Bit("文件"));
	QAction* pActionLoadAeroplane = pMenuFile->addAction(QString::fromLocal8Bit("加载飞行器"));
	QAction* pActionLoadObservedObj = pMenuFile->addAction(QString::fromLocal8Bit("加载观测目标"));

	connect(pActionLoadAeroplane, SIGNAL(triggered()), this, SLOT(slotLoadAeroplane()));
	connect(pActionLoadObservedObj, SIGNAL(triggered()), this, SLOT(slotLoadObservedObj()));

	QMenu* pMenuManipulator = menuBar()->addMenu(QString::fromLocal8Bit("视角模式"));
	QAction* pActionAero = pMenuManipulator->addAction(QString::fromLocal8Bit("飞行器"));
	QAction* pActionTarget = pMenuManipulator->addAction(QString::fromLocal8Bit("目标"));
	QAction* pActionTerrain = pMenuManipulator->addAction(QString::fromLocal8Bit("地面"));
	QAction* pActionAuto = pMenuManipulator->addAction(QString::fromLocal8Bit("自动视角"));

	pActionAero->setCheckable(true);
	pActionTarget->setCheckable(true);
	pActionTerrain->setCheckable(true);
	pActionAuto->setCheckable(true);

	pActionAuto->setChecked(true);

	QActionGroup* pActionGroup = new QActionGroup(pMenuManipulator);
	pActionGroup->addAction(pActionAero);
	pActionGroup->addAction(pActionTarget);
	pActionGroup->addAction(pActionTerrain);
	pActionGroup->addAction(pActionAuto);

	connect(pActionAero, SIGNAL(triggered()), this, SLOT(slotSetManipulatorAeroplane()));
	connect(pActionTarget, SIGNAL(triggered()), this, SLOT(slotSetManipulatorTarget()));
	connect(pActionTerrain, SIGNAL(triggered()), this, SLOT(slotSetManipulatorTerrain()));
	connect(pActionAuto, SIGNAL(triggered()), this, SLOT(slotSetManipulatorAuto()));

	QMenu* pMenuSettings = menuBar()->addMenu(QString::fromLocal8Bit("飞行器姿态"));
	QAction* pActionAeroplanePos = pMenuSettings->addAction(QString::fromLocal8Bit("参数设置"));

	connect(pActionAeroplanePos, SIGNAL(triggered()), this, SLOT(slotSetAeroplaneMatrix()));

	//QMenu* pMenuAeroplanePath = menuBar()->addMenu(QString::fromLocal8Bit("飞行轨迹"));
	//QAction* pCirclePath = pMenuAeroplanePath->addAction(QString::fromLocal8Bit("环形"));
	//QAction* pLinePath = pMenuAeroplanePath->addAction(QString::fromLocal8Bit("直线"));
	//pMenuAeroplanePath->addSeparator();
	//QAction* pActionPathPara = pMenuAeroplanePath->addAction(QString::fromLocal8Bit("参数设置"));

	//pCirclePath->setCheckable(true);
	//pCirclePath->setChecked(true);
	//pLinePath->setCheckable(true);

	//QActionGroup* pActonGroup2 = new QActionGroup(pMenuAeroplanePath);
	//pActonGroup2->addAction(pCirclePath);
	//pActonGroup2->addAction(pLinePath);

	//connect(pCirclePath, SIGNAL(triggered()), this, SLOT(slotCirclePath()));
	//connect(pLinePath, SIGNAL(triggered()), this, SLOT(slotLinePath()));
	//connect(pActionPathPara, SIGNAL(triggered()), this, SLOT(slotPathPara()));

	QMenu* pMenuTargetSettings = menuBar()->addMenu(QString::fromLocal8Bit("目标参数"));
	QAction* pActionTargetSettings = pMenuTargetSettings->addAction(QString::fromLocal8Bit("设置"));

	connect(pActionTargetSettings, SIGNAL(triggered()), this, SLOT(slotTargetPara()));

	//QMenu* pMenuRadarBeamSettings = menuBar()->addMenu(QString::fromLocal8Bit("波束方向"));
	//QAction* pActionRadarBeamSettings = pMenuRadarBeamSettings->addAction(QString::fromLocal8Bit("设置"));

	//connect(pActionRadarBeamSettings, SIGNAL(triggered()), this, SLOT(slotRadarBeamPara()));

	QMenu* pMenuTest = menuBar()->addMenu(QString::fromLocal8Bit("动画"));
	QAction* pActionTest = pMenuTest->addAction(QString::fromLocal8Bit("重置"));
	connect(pActionTest, SIGNAL(triggered()), this, SLOT(slotReset()));

	QAction* pActionPause = pMenuTest->addAction(QString::fromLocal8Bit("暂停"));
	connect(pActionPause, SIGNAL(triggered()), this, SLOT(slotPause()));

	QMenu* pMenuPara = menuBar()->addMenu(QString::fromLocal8Bit("参数"));
	QAction* pActionPara = pMenuPara->addAction(QString::fromLocal8Bit("录屏路径"));
	connect(pActionPara, SIGNAL(triggered()), this, SLOT(slotSetPara()));

	QAction* pActionPlotRange = pMenuPara->addAction(QString::fromLocal8Bit("图表范围"));
	connect(pActionPlotRange, SIGNAL(triggered()), this, SLOT(slotSetPlotRange()));

	QAction* pActionCapture = menuBar()->addAction(QString::fromLocal8Bit("录制"));
	pActionCapture->setCheckable(true);
	pActionCapture->setChecked(false);
	connect(pActionCapture, SIGNAL(triggered()), this, SLOT(slotCapture()));

#if QT_VERSION >= 0x050000
	// Qt5 is currently crashing and reporting "Cannot make QOpenGLContext current in a different thread" when the viewer is run multi-threaded, this is regression from Qt4
	osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::SingleThreaded;
#else
	osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::CullDrawThreadPerContext;
#endif

	ViewerWidget* viewWidget = new ViewerWidget(threadingModel);
	setCentralWidget(viewWidget);

	//return;

	QDockWidget* pDockWidget = new QDockWidget;
	pDockWidget->setWindowTitle(QString::fromLocal8Bit("视频"));
 	VideoPlayer* pPlayer = new VideoPlayer;
	pPlayer->setMinimumHeight(300);
	g_pVideoPlayer = pPlayer;
 	pDockWidget->setWidget(pPlayer);
	
	addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, pDockWidget);
	//pDockWidget->setFloating(true);

	m_pPlot = new Plot(this);
	g_pPlot = m_pPlot;

	//加载曲线显示
	m_pPlot->start();
	//m_pPlot->setIntervalLength(10.0);

	QDockWidget* pDockWidget2 = new QDockWidget;
	pDockWidget2->setWindowTitle("RCS");
	pDockWidget2->setWidget(m_pPlot);
	addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, pDockWidget2);
	//pDockWidget2->setFloating(true);
}

MainWindow3D::~MainWindow3D()
{

}

void MainWindow3D::slotSetPara()
{
	DataManager* pDataManager = DataManager::Instance();
	SetPathDlg dlg(pDataManager->GetScreenCaptureFilePath());

	if (dlg.exec())
	{
		pDataManager->SetScreenCaptureFilePath(dlg.path());
	}
}

void MainWindow3D::slotSetPlotRange()
{
	PlotRangeDlg dlg;
	if (dlg.exec())
	{
		int nHMin = dlg.GetHMin();
		int nHMax = dlg.GetHMax();

		int nVMin = dlg.GetVMin();
		int nVMax = dlg.GetVMax();

		QString strHLabel = dlg.hLabel();
		QString strVLabel = dlg.vLabel();

		m_pPlot->setAxisScale(QwtPlot::yLeft, nVMin, nVMax);
		m_pPlot->setAxisScale(QwtPlot::xBottom, nHMin, nHMax);

		m_pPlot->setAxisTitle(QwtPlot::xBottom, strHLabel);
		m_pPlot->setAxisTitle(QwtPlot::yLeft, strVLabel);

	 	PlotSettings* pSettings = PlotSettings::Instance();

		pSettings->setHLabel(strHLabel);
		pSettings->setVLabel(strVLabel);
		pSettings->setHMinMax(nHMin, nHMax);
		pSettings->setVMinMax(nVMin, nVMax);

		m_pPlot->replot();
	}
}

void MainWindow3D::slotCaptureFinished(int exitCode, QProcess::ExitStatus exitStatus)
{

}

void MainWindow3D::slotCapture()
{
	QString strCapturepath = DataManager::Instance()->GetScreenCaptureFilePath();
	if (strCapturepath.isEmpty())
	{
// 		int ret = QMessageBox::warning(this, QString::fromLocal8Bit("提示"),
// 			QString::fromLocal8Bit("请设置保存路径"),
// 			QMessageBox::Ok);
// 		return;

		DataManager* pDataManager = DataManager::Instance();
		SetPathDlg dlg(pDataManager->GetScreenCaptureFilePath());

		if (dlg.exec())
		{
			pDataManager->SetScreenCaptureFilePath(dlg.path());
			strCapturepath = DataManager::Instance()->GetScreenCaptureFilePath();
		}
		else
		{
			return;
		}
	}

	QFileInfo fileInfo(strCapturepath);
	if (fileInfo.isFile())
		QFile::remove(strCapturepath);

	QAction* pAction = qobject_cast<QAction*>(sender());
	static bool s_bRecording = false;

	if (!s_bRecording)
	{
		int ret = QMessageBox::warning(this, QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("是否开始录制？"),
			QMessageBox::Yes | QMessageBox::No);

		if (ret == QMessageBox::No)
		{
			return;
		}
	}

	s_bRecording = !s_bRecording;

	if (s_bRecording)
	{
		pAction->setText(QString::fromLocal8Bit("停止"));

		if (m_pProcess)
		{
			m_pProcess->kill();
			delete m_pProcess;
			m_pProcess = nullptr;
		}

		QString binPath = QCoreApplication::applicationFilePath();
		QFileInfo info(binPath);
		binPath = info.path();
		binPath += "/ffmpeg.exe";

		m_pProcess = new QProcess(this);
		connect(m_pProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotCaptureFinished(int, QProcess::ExitStatus)));

		//获取窗口要录屏的范围
		QRect rectMenuBar = menuBar()->geometry();
		QWidget* pW = qobject_cast<QWidget*>(menuBar()->parent());
		QPoint ptLT(rectMenuBar.left(), rectMenuBar.bottom());
		ptLT = pW->mapToGlobal(ptLT);

		QRect rectMainWindow = geometry();
		QPoint ptRB(rectMainWindow.right(), rectMainWindow.bottom());

		int nWid = ptRB.x() - ptLT.x();
		int nHei = ptRB.y() - ptLT.y();

		QStringList strList;
		strList << "-f";
		strList << "gdigrab";
		strList << "-framerate";
		strList << "30";
		strList << "-offset_x";
		strList << QString::number(ptLT.x());
		strList << "-offset_y";
		strList << QString::number(ptLT.y());
		strList << "-video_size";
		strList << QString::number(nWid) + 'x' + QString::number(nHei);
// 		strList << "-show_region";
// 		strList << "1";
		strList << "-i";
		strList << "desktop";
		strList << strCapturepath/*"output11.mkv"*/;

		//ffmpeg -f gdigrab -framerate 30 -offset_x 10 -offset_y 20 -video_size 640x480 -show_region 1 -i desktop output.mkv

		m_pProcess->start(binPath, strList);
		if (!m_pProcess->waitForStarted())
		{
			delete m_pProcess;
			m_pProcess = nullptr;
			return;
		}
	}
	else
	{
		pAction->setText(QString::fromLocal8Bit("录制"));

		if (m_pProcess)
		{
			m_pProcess->write("q");
			m_pProcess->waitForFinished();
			delete m_pProcess;
			m_pProcess = nullptr;
		}
	}
}

void MainWindow3D::slotSetManipulatorAeroplane()
{
	QAction* pAction = qobject_cast<QAction*>(sender());
	pAction->setChecked(true);

	SetNodeTrackerManipulator();
}

void MainWindow3D::slotSetManipulatorTarget()
{
	QAction* pAction = qobject_cast<QAction*>(sender());
	pAction->setChecked(true);

	SetNodeTrackerManipulator(1);
}

void MainWindow3D::slotSetManipulatorTerrain()
{
	QAction* pAction = qobject_cast<QAction*>(sender());
	pAction->setChecked(true);

	SetTerrainManipulator();
}

void MainWindow3D::slotSetManipulatorAuto()
{
	QAction* pAction = qobject_cast<QAction*>(sender());
	pAction->setChecked(true);

	double dx1, dy1, dx2, dy2, dH;
	DataManager::Instance()->GetPlanePathEnv(dx1, dy1, dx2, dy2, dH);
	SetAutoManipulator(dx1, dy2, dx2, dy1, dH);
}

void MainWindow3D::slotLoadAeroplane()
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

void MainWindow3D::slotLoadObservedObj()
{
	QString strFileName = QFileDialog::getOpenFileName();
	if (strFileName.isEmpty())
		return;

	DataManager::Instance()->LoadTargetObject(strFileName);
}

void MainWindow3D::slotSetAeroplaneMatrix()
{
	AeroRotateDlg dlg;
	dlg.exec();
}

void MainWindow3D::slotCirclePath()
{
	QAction* pAction = qobject_cast<QAction*>(sender());
	pAction->setChecked(true);

	DataManager::Instance()->SetPathType(DataManager::PathType_Circle);
}

void MainWindow3D::slotLinePath()
{
	QAction* pAction = qobject_cast<QAction*>(sender());
	pAction->setChecked(true);

	DataManager::Instance()->SetPathType(DataManager::PathType_Line);
}

void MainWindow3D::slotPathPara()
{
	PathSettingsDlg dlg;
	dlg.exec();
}

void MainWindow3D::slotTargetPara()
{
	TargetSettingsDlg dlg;
	dlg.exec();
}

void MainWindow3D::slotRadarBeamPara()
{
	RadarBeamRotateDlg dlg;
	dlg.exec();
}

void MainWindow3D::slotReset()
{
	SignalData::instance().Clear();

	//加载曲线显示
	if (g_pSampleThread)
	{
		if (g_pSampleThread->isRunning())
		{
			g_pSampleThread->Cancel();
			//g_pSampleThread->wait(1000);
		}

		delete g_pSampleThread;
		g_pSampleThread = nullptr;
	}

	g_pSampleThread = new SamplingThread;

	g_pSampleThread->setFrequency(0.05);
	g_pSampleThread->setAmplitude(40);
	g_pSampleThread->setInterval(100);

	g_pSampleThread->start();

	m_pPlot->start();
	m_pPlot->replot();

	DataManager* pDataManager = DataManager::Instance();
	pDataManager->ClearPlanePathLine();
	pDataManager->ResetAnimationPath();
}
 
void MainWindow3D::slotPause()
{
	DataManager* pDataManager = DataManager::Instance();
	pDataManager->PauseAnimation();
}