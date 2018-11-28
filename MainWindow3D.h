#ifndef MAINWINDOW3D_H
#define MAINWINDOW3D_H

#include <QtWidgets/QMainWindow>

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

	void slotLoadAeroplane();

	void slotLoadObservedObj();

	void slotSetAeroplaneMatrix();

	void slotCirclePath();

	void slotLinePath();

	void slotPathPara();

	void slotTargetPara();

	void slotRadarBeamPara();

// private:
// 	Ui::MainWindowClass ui;
};

#endif // MAINWINDOW3D_H
