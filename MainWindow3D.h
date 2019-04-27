#ifndef MAINWINDOW3D_H
#define MAINWINDOW3D_H

#include <QtWidgets/QMainWindow>
#include "plot.h"

class MainWindow3D : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow3D(QWidget *parent = 0);
	~MainWindow3D();

public slots:

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

	void slotTest();

private:

	Plot* m_pPlot;

// private:
// 	Ui::MainWindowClass ui;
};

#endif // MAINWINDOW3D_H
