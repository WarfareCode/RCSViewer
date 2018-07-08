#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:

	void slotSetManipulatorAeroplane();

	void slotSetManipulatorTerrain();

	void slotLoadAeroplane();

	void slotLoadObservedObj();

	void slotSetAeroplaneMatrix();

private:
	Ui::MainWindowClass ui;
};

#endif // MAINWINDOW_H
