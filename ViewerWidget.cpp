#include "ViewerWidget.h"
#include "DataManager.h"

osgViewer::View* g_pView = nullptr;

void SetTerrainManipulator()
{
	DataManager* pManager = DataManager::Instance();
	osgGA::TerrainManipulator* pManipulator = new osgGA::TerrainManipulator;
	pManipulator->setNode(pManager->GetTerrainNode());
	g_pView->setCameraManipulator(pManipulator);
}

void SetNodeTrackerManipulator()
{
	DataManager* pManager = DataManager::Instance();
	osgGA::NodeTrackerManipulator* pManipulator = new osgGA::NodeTrackerManipulator;
	pManipulator->setTrackNode(pManager->GetAerocraftNode());
	g_pView->setCameraManipulator(pManipulator/*new osgGA::TrackballManipulator*/);
}

ViewerWidget::ViewerWidget(osgViewer::ViewerBase::ThreadingModel threadingModel) : QWidget()
{
	setThreadingModel(threadingModel);

	// disable the default setting of viewer.done() by pressing Escape.
	setKeyEventSetsDone(0);

	osgQt::GraphicsWindowQt* gw = createGraphicsWindow(0, 0, 100, 100);
	osgViewer::View* view = new osgViewer::View;

	g_pView = view;
	addView(view);

	osg::Camera* camera = view->getCamera();
	camera->setGraphicsContext(gw);

	const osg::GraphicsContext::Traits* traits = gw->getTraits();

	camera->setClearColor(osg::Vec4(0.2, 0.2, 0.6, 1.0));
	camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
	camera->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(traits->width) / static_cast<double>(traits->height), 1.0f, 10000.0f);

	DataManager* pManager = DataManager::Instance();
	pManager->LoadTerrain();
	pManager->LoadAerocraft("c:/a/zz.FBX");

	view->setSceneData(pManager->GetRootNode());
	view->addEventHandler(new osgViewer::StatsHandler);

	SetNodeTrackerManipulator();

	QWidget* widget1 = gw->getGLWidget();

	//         QWidget* widget2 = addViewWidget( createGraphicsWindow(0,0,100,100), osgDB::readNodeFile("glider.osgt") );
	//         QWidget* widget3 = addViewWidget( createGraphicsWindow(0,0,100,100), osgDB::readNodeFile("axes.osgt") );
	//         QWidget* widget4 = addViewWidget( createGraphicsWindow(0,0,100,100), osgDB::readNodeFile("fountain.osgt") );
	//         QWidget* popupWidget = addViewWidget( createGraphicsWindow(900,100,320,240,"Popup window",true), osgDB::readNodeFile("dumptruck.osgt") );
	//         popupWidget->show();

	QGridLayout* grid = new QGridLayout;
	grid->addWidget(widget1, 0, 0);
	//      grid->addWidget( widget2, 0, 1 );
	//      grid->addWidget( widget3, 1, 0 );
	//      grid->addWidget( widget4, 1, 1 );
	setLayout(grid);

	connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
	_timer.start(10);
}

QWidget* ViewerWidget::addViewWidget(osgQt::GraphicsWindowQt* gw, osg::Node* scene, osg::Node* pNode)
{
	osgViewer::View* view = new osgViewer::View;
	addView(view);

	osg::Camera* camera = view->getCamera();
	camera->setGraphicsContext(gw);

	const osg::GraphicsContext::Traits* traits = gw->getTraits();

	camera->setClearColor(osg::Vec4(0.2, 0.2, 0.6, 1.0));
	camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
	camera->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(traits->width) / static_cast<double>(traits->height), 1.0f, 10000.0f);

	view->setSceneData(scene);
	view->addEventHandler(new osgViewer::StatsHandler);
	osgGA::TerrainManipulator* pManipulator = new osgGA::TerrainManipulator;
	pManipulator->setNode(pNode);
	view->setCameraManipulator(pManipulator/*new osgGA::TrackballManipulator*/);

	return gw->getGLWidget();
}

osgQt::GraphicsWindowQt* ViewerWidget::createGraphicsWindow(int x, int y, int w, int h, const std::string& name, bool windowDecoration)
{
	osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	traits->windowName = name;
	traits->windowDecoration = windowDecoration;
	traits->x = x;
	traits->y = y;
	traits->width = w;
	traits->height = h;
	traits->doubleBuffer = true;
	traits->alpha = ds->getMinimumNumAlphaBits();
	traits->stencil = ds->getMinimumNumStencilBits();
	traits->sampleBuffers = ds->getMultiSamples();
	traits->samples = ds->getNumMultiSamples();

	return new osgQt::GraphicsWindowQt(traits.get());
}