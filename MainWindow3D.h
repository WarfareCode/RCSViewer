#ifndef MAINWINDOW3D_H
#define MAINWINDOW3D_H

#include <QtWidgets/QMainWindow>
#include "plot.h"
#include <QtCore/QProcess>
class  QStackedWidget;

class MainWindow3D : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow3D(QWidget *parent = 0);
	~MainWindow3D();

	void show(bool bJPG);

public slots:

	void slotSetManipulatorSide();

	void slotSetManipulatorAeroplane();

	void slotSetManipulatorTarget();

	void slotSetManipulatorTerrain();

	void slotSetManipulatorAuto();

	void slotLoadAeroplane();

	void slotLoadObservedObj();

	void slotSetAeroplaneMatrix();

	void slotCirclePath();

	void slotLinePath();

	void slotPathPara();

	void slotTargetPara();

	void slotRadarBeamPara();

	void slotReset();

	void slotPause();

	void slotSetPara();

	void slotSetPlotRange();

	void slotCapture();

	void slotCaptureFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:

	Plot* m_pPlot;

	QProcess* m_pProcess;

	QStackedWidget* m_pStackedWidget;

// private:
// 	Ui::MainWindowClass ui;
};

#endif // MAINWINDOW3D_H
