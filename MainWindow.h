#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_Mainwindow.h"
#include "FileTreeView.h"

#include <QtitanRibbon.h>


namespace Qtitan
{
	class WidgetGalleryItems;
	class OfficeWidgetGallery;
}

class MainWindow : public Qtitan::RibbonMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:

	void slotOpenProject();
	void slotNewProject();

	void slotRencentProject();

public:

	void loadRencentProjectConfig();

	void saveRencentProjectConfig();

protected:

	void addRecentProject(const QString& strProjectPath);

	void deleteRecentProject(const QString& strProjectPath);

private:
	//Ui::Mainwindow ui;

	FileTreeView* m_pFileTreeView;
	CentralWidget* m_pCentralWidget;

private:
	void createMenuFile();
	void createRibbon();
	void createStatusBar();
	void createOptions();

protected Q_SLOTS:

	void optionsTheme(QAction*);
	void pressButton();
	void maximizeToggle();
	void minimizationChanged(bool minimized);
	void optionsFont(QAction* act);
	void setDPIToggled(bool);

private:
	QAction* m_actionRibbonMinimize;
	QAction* m_actionDefault;
	QAction* m_actionNormal;
	QAction* m_actionLarge;
	QAction* m_actionExLarge;

	int m_defaultFont;

	Qtitan::RibbonStyle* m_ribbonStyle;

	QMenu* m_pRencentMenu;

	QStringList m_listRecentProject;
};

#endif // MAINWINDOW_H
