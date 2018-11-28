#include "Mainwindow.h"
#include "NewProjectDlg.h"
#include "OpenProjectDlg.h"
#include <QtWidgets/QDockWidget>
#include <QTextCodec>

#include <CentralWidget.h>
#include <QtWidgets/QFileDialog>
#include "Document.h"
#include <QCoreApplication>

////////////////////////////////////////////
#include <QFileDialog>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QStyleOption>
#include <QPaintEvent>
#include <QJsonDocument>
#include <QTextStream>
#include <QJsonArray>
#include <QCoreApplication>
#include <QMessageBox>

#include "QtnRibbonDef.h"
#include "QtnRibbonQuickAccessBar.h"


#define Font_Default        "System Default"
#define Font_Normal         "Normal"
#define Font_Large          "Large"
#define Font_ExLarge        "Extra Large"
#define DPI_100_percent     "DPI 100%"

MainWindow::MainWindow(QWidget *parent)
	: Qtitan::RibbonMainWindow(parent)
{
	m_ribbonStyle = qobject_cast<Qtitan::RibbonStyle*>(qApp->style());
	m_ribbonStyle->setTheme(OfficeStyle::Office2016Colorful);

	m_defaultFont = 8;

	setWindowTitle(QString::fromLocal8Bit("文件管理"));
	m_pFileTreeView = new FileTreeView(0);
	
	QDockWidget* pDockWidget = new QDockWidget(this);
	pDockWidget->setWidget(m_pFileTreeView);
	
	m_pCentralWidget = new CentralWidget;
	m_pFileTreeView->setBuddy(m_pCentralWidget);
	setCentralWidget(m_pCentralWidget);
	
	addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea , pDockWidget);

	ribbonBar()->setFrameThemeEnabled();
	ribbonBar()->setTitleBarVisible(false);
	
	createMenuFile();
	createRibbon();
	statusBar();

	createOptions();

	m_actionRibbonMinimize = ribbonBar()->addAction(QIcon(QCoreApplication::applicationDirPath() + "/res/ribbonMaximize.png"), "Minimize the Ribbon", Qt::ToolButtonIconOnly);
	ribbonBar()->setMinimized(true);
	m_actionRibbonMinimize->setStatusTip(tr("Show only the tab names on the Ribbon"));
	m_actionRibbonMinimize->setShortcut(tr("Ctrl+F1"));
	connect(m_actionRibbonMinimize, SIGNAL(triggered()), this, SLOT(maximizeToggle()));
	connect(ribbonBar(), SIGNAL(minimizationChanged(bool)), this, SLOT(minimizationChanged(bool)));

	QRect geom = QApplication::desktop()->availableGeometry();
	move(QPoint(200, 200));
	resize(2 * geom.width() / 3, 2 * geom.height() / 3);
}

MainWindow::~MainWindow()
{

}

void MainWindow::addRecentProject(const QString& strProjectPath)
{
	if (m_listRecentProject.contains(strProjectPath))
	{
		m_listRecentProject.removeOne(strProjectPath);
	}

	m_listRecentProject.push_front(strProjectPath);

	if (m_listRecentProject.size() > 10)
		m_listRecentProject.pop_back();

	m_pRencentMenu->clear();
	for (int i = 0; i < m_listRecentProject.size(); i ++)
	{
		QAction* pAction = m_pRencentMenu->addAction(m_listRecentProject[i]);
		connect(pAction, SIGNAL(triggered()), this, SLOT(slotRencentProject()));
	}
}

void MainWindow::deleteRecentProject(const QString& strProjectPath)
{
	if (m_listRecentProject.contains(strProjectPath))
	{
		m_listRecentProject.removeOne(strProjectPath);

		m_pRencentMenu->clear();
		for (int i = 0; i < m_listRecentProject.size(); i++)
		{
			QAction* pAction = m_pRencentMenu->addAction(m_listRecentProject[i]);
			connect(pAction, SIGNAL(triggered()), this, SLOT(slotRencentProject()));
		}
	}
}

void MainWindow::slotOpenProject()
{
	OpenProjectDlg dlg;
	if (dlg.exec())
	{
		QString strPrjFile = QFileDialog::getOpenFileName(this, tr("Open File"),
			"/home", tr("Project (*.rcsprj)"));

		if (strPrjFile.isEmpty() || strPrjFile.isNull())
			return;

		QFileInfo fileInfo(strPrjFile);
		if (!Document::openProject(fileInfo.path()))
			return;

		addRecentProject(strPrjFile);
		m_pFileTreeView->setWorkSpace(Document::typeDir());
		m_pCentralWidget->changeDatabase();
	}
}

void MainWindow::slotNewProject()
{
	NewProjectDlg dlg;
	if (dlg.exec())
	{
		if (!Document::newProject(dlg.projectDir()))
			return;

		QString strProjectFilePath = dlg.projectDir() + "/" + dlg.projectName();
		addRecentProject(strProjectFilePath);

		QFile file(strProjectFilePath);
		if (file.open(QIODevice::WriteOnly))
		{
			QTextStream stream(&file);
			stream.setCodec(QTextCodec::codecForName("UTF-8"));
			stream << "data.db";
		}

		m_pFileTreeView->setWorkSpace(Document::typeDir());
		m_pCentralWidget->changeDatabase();
	}
}

void MainWindow::slotRencentProject()
{
	QAction* pAction = (QAction*)sender();
	QString strDir = pAction->text();
	
	QFileInfo fileInfo(strDir);

	if (!Document::openProject(fileInfo.path()))
	{
		int ok = QMessageBox::warning(this, QStringLiteral("提示"),
			QStringLiteral("工程打开失败，是否从列表删除该工程？"),
			QMessageBox::Yes, QMessageBox::No);

		if (ok == QMessageBox::Yes)
		{
			deleteRecentProject(strDir);
		}
		
		return;
	}

	addRecentProject(strDir);
	m_pFileTreeView->setWorkSpace(Document::typeDir());
	m_pCentralWidget->changeDatabase();
}

void MainWindow::loadRencentProjectConfig()
{
	QFile file(QCoreApplication::applicationDirPath() + "/rencent.txt");
	if (file.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream in(&file);
		in.setCodec(QTextCodec::codecForName("UTF-8"));
		QString myText = in.readAll();
		file.close();

		QByteArray byteArray = myText.toUtf8();
		QJsonDocument jsonDocument = QJsonDocument::fromJson(byteArray);
		QJsonArray jsonArray = jsonDocument.array();

		for (int i = 0; i < jsonArray.size(); i++)
		{
			m_listRecentProject.push_back(jsonArray.at(i).toString());
		}

		m_pRencentMenu->clear();
		for (int i = 0; i < m_listRecentProject.size(); i++)
		{
			QAction* pAction = m_pRencentMenu->addAction(m_listRecentProject[i]);
			connect(pAction, SIGNAL(triggered()), this, SLOT(slotRencentProject()));
		}
	}
}

void MainWindow::saveRencentProjectConfig()
{
	QJsonArray jsonArray;

	for (int i = 0; i < m_listRecentProject.size(); i++)
	{
		jsonArray.append(QJsonValue(m_listRecentProject[i]));
	}

	QJsonDocument jsonDocument;
	jsonDocument.setArray(jsonArray);
	QByteArray byteArray = jsonDocument.toJson();

	QFile file(QCoreApplication::applicationDirPath() + "/rencent.txt");
	// Trying to open in WriteOnly and Text mode
	if (file.open(QFile::WriteOnly | QFile::Text))
	{
		QTextStream out(&file);
		out.setCodec(QTextCodec::codecForName("UTF-8"));
		out << byteArray;
		file.close();
	}
}

void MainWindow::createMenuFile()
{
	QIcon iconLogo;
	iconLogo.addPixmap(QPixmap(QCoreApplication::applicationDirPath() + "/res/qtitan.png"));
	iconLogo.addPixmap(QPixmap(QCoreApplication::applicationDirPath() + "/res/qtitanlogo32x32.png"));
	if (QAction* actionFile = ribbonBar()->addSystemButton(iconLogo, QString::fromLocal8Bit("文件")))
	{
		actionFile->setToolTip(QString::fromLocal8Bit("打开或者关闭工程"));

		if (Qtitan::RibbonSystemPopupBar* popupBar = qobject_cast<Qtitan::RibbonSystemPopupBar*>(actionFile->menu()))
		{
			QAction* pActionNewProject = popupBar->addAction(QIcon(QCoreApplication::applicationDirPath() + "/res/new.png"), QString::fromLocal8Bit("新建工程"));
			QAction* pActionOpenProject = popupBar->addAction(QIcon(QCoreApplication::applicationDirPath() + "/res/open.png"), QString::fromLocal8Bit("打开工程"));

			connect(pActionNewProject, SIGNAL(triggered()), this, SLOT(slotNewProject()));
			connect(pActionOpenProject, SIGNAL(triggered()), this, SLOT(slotOpenProject()));

			m_pRencentMenu = popupBar->addMenu(QString::fromLocal8Bit("最近文档"));

			QAction* pActionClose = popupBar->addAction(QIcon(QCoreApplication::applicationDirPath() + "/res/close.png"), QString::fromLocal8Bit("退出"));
			connect(pActionClose, SIGNAL(triggered()), this, SLOT(close()));
		}
	}
}

void MainWindow::createRibbon()
{
	if (Qtitan::RibbonPage* pageButtons = ribbonBar()->addPage(tr("&Home")))
	{
// 		if (Qtitan::RibbonGroup* groupClipboard = pageButtons->addGroup(tr("Clipboard")))
// 		{
// 			QMenu* editPaste = new QMenu(this);
// 			QAction* actionPaste1 = editPaste->addAction(QIcon(":/res/smallClipboard.png"), tr("Paste"));
// 			actionPaste1->setPriority(QAction::LowPriority);
// 			actionPaste1->setShortcut(QKeySequence::Paste);
// 
// 			QAction* actionPaste2 = groupClipboard->addAction(QIcon(":/res/clipboard.png"),
// 				tr("&Paste"), Qt::ToolButtonTextUnderIcon, editPaste);
// 			actionPaste2->setPriority(QAction::LowPriority);
// 			actionPaste2->setShortcut(QKeySequence::Paste);
// 
// 			QAction* actionCut = groupClipboard->addAction(QIcon(":/res/smallCut.png"),
// 				tr("&Cut"), Qt::ToolButtonTextBesideIcon);
// 			actionCut->setShortcut(QKeySequence::Cut);
// 
// 			QAction* actionCopy = groupClipboard->addAction(QIcon(":/res/smallCopy.png"),
// 				tr("&Copy"), Qt::ToolButtonTextBesideIcon);
// 			actionCopy->setShortcut(QKeySequence::Copy);
// 
// 			groupClipboard->addAction(QIcon(":/res/smallFormatPainter.png"),
// 				tr("F&ormat Pointer"), Qt::ToolButtonTextBesideIcon);
// 
// 			groupClipboard->setEnabled(false);
// 		}
	}
	m_defaultFont = ribbonBar()->font().pointSize();

	if (QToolButton* button = ribbonBar()->getSystemButton())
		button->setToolButtonStyle(Qt::ToolButtonTextOnly);
}

void MainWindow::createOptions()
{
	Qtitan::RibbonStyle::Theme themeId = Qtitan::RibbonStyle::Office2007Blue;
	if (m_ribbonStyle)
		themeId = m_ribbonStyle->getTheme();

	QMenu* menu = ribbonBar()->addMenu(tr("Options"));
	QAction* actionStyle = menu->addAction(tr("Style"));

	QMenu* menuStyle = new QMenu(ribbonBar());
	QActionGroup* styleActions = new QActionGroup(this);

	QAction* actionBlue = menuStyle->addAction(tr("Office 2007 Blue"));
	actionBlue->setCheckable(true);
	actionBlue->setChecked(themeId == Qtitan::RibbonStyle::Office2007Blue);
	actionBlue->setObjectName("Office2007Blue");

	QAction* actionBlack = menuStyle->addAction(tr("Office 2007 Black"));
	actionBlack->setObjectName("Office2007Black");
	actionBlack->setCheckable(true);
	actionBlack->setChecked(themeId == Qtitan::RibbonStyle::Office2007Black);

	QAction* actionSilver = menuStyle->addAction(tr("Office 2007 Silver"));
	actionSilver->setObjectName("Office2007Silver");
	actionSilver->setCheckable(true);
	actionSilver->setChecked(themeId == Qtitan::RibbonStyle::Office2007Silver);

	QAction* actionAqua = menuStyle->addAction(tr("Office 2007 Aqua"));
	actionAqua->setObjectName("Office2007Aqua");
	actionAqua->setCheckable(true);
	actionAqua->setChecked(themeId == Qtitan::RibbonStyle::Office2007Aqua);

	menuStyle->addSeparator();

	QAction* actionScenic = menuStyle->addAction(tr("Windows 7 Scenic"));
	actionScenic->setObjectName("Windows7Scenic");
	actionScenic->setCheckable(true);
	actionScenic->setChecked(themeId == Qtitan::RibbonStyle::Windows7Scenic);

	menuStyle->addSeparator();

	QAction* action2010Blue = menuStyle->addAction(tr("Office 2010 Blue"));
	action2010Blue->setObjectName("Office2010Blue");
	action2010Blue->setCheckable(true);
	action2010Blue->setChecked(themeId == Qtitan::RibbonStyle::Office2010Blue);

	QAction* action2010Silver = menuStyle->addAction(tr("Office 2010 Silver"));
	action2010Silver->setObjectName("Office2010Silver");
	action2010Silver->setCheckable(true);
	action2010Silver->setChecked(themeId == Qtitan::RibbonStyle::Office2010Silver);

	QAction* action2010Black = menuStyle->addAction(tr("Office 2010 Black"));
	action2010Black->setObjectName("Office2010Black");
	action2010Black->setCheckable(true);
	action2010Black->setChecked(themeId == Qtitan::RibbonStyle::Office2010Black);

	menuStyle->addSeparator();

	QAction* action2016Colorful = menuStyle->addAction(tr("Office 2016 Colorful"));
	action2016Colorful->setObjectName("Office2016Colorful");
	action2016Colorful->setCheckable(true);
	action2016Colorful->setChecked(themeId == Qtitan::RibbonStyle::Office2016Colorful);

	QAction* action2016White = menuStyle->addAction(tr("Office 2016 White"));
	action2016White->setObjectName("Office2016White");
	action2016White->setCheckable(true);
	action2016White->setChecked(themeId == Qtitan::RibbonStyle::Office2016DarkGray);

	QAction* action2016DarkGray = menuStyle->addAction(tr("Office 2016 Dark Gray"));
	action2016DarkGray->setObjectName("Office2016DarkGray");
	action2016DarkGray->setCheckable(true);
	action2016DarkGray->setChecked(themeId == Qtitan::RibbonStyle::Office2016DarkGray);

	styleActions->addAction(actionBlue);
	styleActions->addAction(actionBlack);
	styleActions->addAction(actionSilver);
	styleActions->addAction(actionAqua);
	styleActions->addAction(actionScenic);
	styleActions->addAction(action2010Blue);
	styleActions->addAction(action2010Silver);
	styleActions->addAction(action2010Black);
	styleActions->addAction(action2016Colorful);
	styleActions->addAction(action2016White);
	styleActions->addAction(action2016DarkGray);

	actionStyle->setMenu(menuStyle);
	connect(styleActions, SIGNAL(triggered(QAction*)), this, SLOT(optionsTheme(QAction*)));

	QAction* actionMenu = menu->addAction(tr("Font"));

	QMenu* menuFont = new QMenu(ribbonBar());
	QActionGroup* fontActions = new QActionGroup(this);

	m_actionDefault = menuFont->addAction(Font_Default);
	m_actionDefault->setCheckable(true);
	m_actionDefault->setChecked(true);
	m_actionDefault->setObjectName(Font_Default);
	fontActions->addAction(m_actionDefault);

	menuFont->addSeparator();

	m_actionNormal = menuFont->addAction(Font_Normal);
	m_actionNormal->setCheckable(true);
	m_actionNormal->setObjectName(Font_Normal);
	fontActions->addAction(m_actionNormal);

	m_actionLarge = menuFont->addAction(Font_Large);
	m_actionLarge->setCheckable(true);
	m_actionLarge->setObjectName(Font_Large);
	fontActions->addAction(m_actionLarge);

	m_actionExLarge = menuFont->addAction(Font_ExLarge);
	m_actionExLarge->setCheckable(true);
	m_actionExLarge->setObjectName(Font_ExLarge);
	fontActions->addAction(m_actionExLarge);
	actionMenu->setMenu(menuFont);
	connect(fontActions, SIGNAL(triggered(QAction*)), this, SLOT(optionsFont(QAction*)));

	menuFont->addSeparator();
	QAction* actionDPI_100_percent = menuFont->addAction(tr("DPI Aware (new)"));
	actionDPI_100_percent->setObjectName(DPI_100_percent);
	actionDPI_100_percent->setCheckable(true);
	actionDPI_100_percent->setChecked(true);
	connect(actionDPI_100_percent, SIGNAL(toggled(bool)), this, SLOT(setDPIToggled(bool)));

	menu->addSeparator();
	QAction* actionCusomize = menu->addAction(tr("Cusomize..."));
	actionCusomize->setEnabled(false);
}

void MainWindow::optionsTheme(QAction* action)
{
	if (m_ribbonStyle)
	{
		Qtitan::RibbonStyle::Theme themeId = Qtitan::RibbonStyle::Office2007Blue;
		if (action->objectName() == "Office2007Black")
			themeId = Qtitan::RibbonStyle::Office2007Black;
		else if (action->objectName() == "Office2007Silver")
			themeId = Qtitan::RibbonStyle::Office2007Silver;
		else if (action->objectName() == "Office2007Aqua")
			themeId = Qtitan::RibbonStyle::Office2007Aqua;
		else if (action->objectName() == "Windows7Scenic")
			themeId = Qtitan::OfficeStyle::Windows7Scenic;
		else if (action->objectName() == "Office2010Blue")
			themeId = Qtitan::OfficeStyle::Office2010Blue;
		else if (action->objectName() == "Office2010Silver")
			themeId = Qtitan::OfficeStyle::Office2010Silver;
		else if (action->objectName() == "Office2010Black")
			themeId = Qtitan::OfficeStyle::Office2010Black;
		else if (action->objectName() == "Office2016Colorful")
			themeId = Qtitan::OfficeStyle::Office2016White;
		else if (action->objectName() == "Office2016White")
			themeId = Qtitan::OfficeStyle::Office2016White;
		else if (action->objectName() == "Office2016DarkGray")
			themeId = Qtitan::OfficeStyle::Office2016DarkGray;

		if (QToolButton* button = ribbonBar()->getSystemButton())
		{
			if (themeId == Qtitan::OfficeStyle::Windows7Scenic ||
				themeId == Qtitan::OfficeStyle::Office2010Blue ||
				themeId == Qtitan::OfficeStyle::Office2010Silver ||
				themeId == Qtitan::OfficeStyle::Office2010Black ||
				themeId == Qtitan::OfficeStyle::Office2016Colorful ||
				themeId == Qtitan::OfficeStyle::Office2016White ||
				themeId == Qtitan::OfficeStyle::Office2016DarkGray)
				button->setToolButtonStyle(Qt::ToolButtonTextOnly);
			else
				button->setToolButtonStyle(Qt::ToolButtonFollowStyle);
		}

		m_ribbonStyle->setTheme(themeId);
		m_actionDefault->setChecked(true);
	}
}

void MainWindow::pressButton()
{

}

void MainWindow::maximizeToggle()
{
	ribbonBar()->setMinimized(!ribbonBar()->isMinimized());
}

void MainWindow::minimizationChanged(bool minimized)
{
	m_actionRibbonMinimize->setChecked(minimized);
	m_actionRibbonMinimize->setIcon(minimized ? QIcon(QCoreApplication::applicationDirPath() + "/res/ribbonMaximize.png") : QIcon(QCoreApplication::applicationDirPath() + "/res/ribbonMinimize.png"));
}

void MainWindow::optionsFont(QAction* act)
{
	QFont fnt = ribbonBar()->font();

	if (Font_Default == act->objectName())
		fnt.setPointSize(m_defaultFont);
	else if (Font_Normal == act->objectName())
		fnt.setPointSize(8);
	else if (Font_Large == act->objectName())
		fnt.setPointSize(11);
	else if (Font_ExLarge == act->objectName())
		fnt.setPointSize(13);

	ribbonBar()->setFont(fnt);
}

void MainWindow::setDPIToggled(bool on)
{
	m_actionDefault->setEnabled(on);
	m_actionNormal->setEnabled(on);
	m_actionLarge->setEnabled(on);
	m_actionExLarge->setEnabled(on);
	m_ribbonStyle->setDPIAware(on);
}