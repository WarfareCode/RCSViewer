#include "ViewerWidget.h"
#include "DataManager.h"
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osg/CullSettings>

#include <osgOcean/OceanScene>
#include <osgOcean/Version>
#include <osgOcean/ShaderManager>

#include "Scene.h"

osgViewer::View* g_pView = nullptr;

class CameraTrackCallback : public osg::NodeCallback
{
public:
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		if (nv->getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
		{
			osgUtil::CullVisitor* cv = static_cast<osgUtil::CullVisitor*>(nv);
			osg::Vec3f centre, up, eye;
			// get MAIN camera eye,centre,up
			cv->getRenderStage()->getCamera()->getViewMatrixAsLookAt(eye, centre, up);
			// update position
			osg::MatrixTransform* mt = static_cast<osg::MatrixTransform*>(node);
			mt->setMatrix(osg::Matrix::translate(eye.x(), eye.y(), mt->getMatrix().getTrans().z()));
		}

		traverse(node, nv);
	}
};

osg::ref_ptr<osg::TextureCubeMap> loadCubeMapTextures(const std::string& dir)
{
	enum { POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z };

	std::string filenames[6];

	filenames[POS_X] = "textures/" + dir + "/east.png";
	filenames[NEG_X] = "textures/" + dir + "/west.png";
	filenames[POS_Z] = "textures/" + dir + "/north.png";
	filenames[NEG_Z] = "textures/" + dir + "/south.png";
	filenames[POS_Y] = "textures/" + dir + "/down.png";
	filenames[NEG_Y] = "textures/" + dir + "/up.png";

	osg::ref_ptr<osg::TextureCubeMap> cubeMap = new osg::TextureCubeMap;
	cubeMap->setInternalFormat(GL_RGBA);

	cubeMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	cubeMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	cubeMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	cubeMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

	cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_X, osgDB::readImageFile(filenames[NEG_X]));
	cubeMap->setImage(osg::TextureCubeMap::POSITIVE_X, osgDB::readImageFile(filenames[POS_X]));
	cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Y, osgDB::readImageFile(filenames[NEG_Y]));
	cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Y, osgDB::readImageFile(filenames[POS_Y]));
	cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Z, osgDB::readImageFile(filenames[NEG_Z]));
	cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Z, osgDB::readImageFile(filenames[POS_Z]));

	return cubeMap;
}

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
	pManipulator->setMinimumDistance(0.002);
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

	//camera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
	camera->setNearFarRatio(0.0002f);
	//camera->setClearColor(osg::Vec4(0.46, 0.59, 0.71, 1.0));

	DataManager* pManager = DataManager::Instance();
	pManager->LoadTerrain();
	pManager->LoadAerocraft("c:/a/plane.FBX");

	{
		osg::ref_ptr<osgOcean::FFTOceanSurface> surface = new osgOcean::FFTOceanSurface(64, 256, 17
			, osg::Vec2(1.1f, 1.1f), 12, 10, 0.8, 1e-8, true, 2.5, 20.0, 256);
		surface->setFoamBottomHeight(2.2);
		surface->setFoamTopHeight(3.0);
		surface->enableCrestFoam(true);

		osg::PositionAttitudeTransform* pTransform = new osg::PositionAttitudeTransform;
		pTransform->addChild(surface.get());
		pTransform->setScale(osg::Vec3d(0.001, 0.001, 0.001));
		pTransform->setPosition(osg::Vec3d(121.1, 23.7, -0.004));

		osg::Group* pRoot = dynamic_cast<osg::Group*>(pManager->GetRootNode());
		pRoot->addChild(pTransform);
	}

	//天空盒
	if (1)
	{
		std::vector<std::string> _cubemapDirs;
		_cubemapDirs.push_back("sky_clear");
		_cubemapDirs.push_back("sky_dusk");
		_cubemapDirs.push_back("sky_fair_cloudy");

		osg::ref_ptr<osg::TextureCubeMap> _cubemap;
		_cubemap = loadCubeMapTextures(_cubemapDirs[1]);

		osg::ref_ptr<SkyDome> _skyDome;
		_skyDome = new SkyDome(19.f, 16, 16, _cubemap.get());

		osg::Group* pRoot = dynamic_cast<osg::Group*>(pManager->GetRootNode());

		osg::MatrixTransform* transform = new osg::MatrixTransform;
		transform->setDataVariance(osg::Object::DYNAMIC);
		transform->setMatrix(osg::Matrixf::translate(osg::Vec3f(0.f, 0.f, 0.f)));
		transform->setCullCallback(new CameraTrackCallback);

		transform->addChild(_skyDome.get());

		pRoot->addChild(transform);
	}

	view->setSceneData(pManager->GetRootNode());
	view->addEventHandler(new osgViewer::StatsHandler);

	view->getLight()->setPosition(osg::Vec4(1.0f, 1.0f, 1.0f, 0.0f));
	// 环境光
	view->getLight()->setAmbient(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	// 漫反射光
	view->getLight()->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

	SetNodeTrackerManipulator();
	//SetTerrainManipulator();

	QWidget* widget1 = gw->getGLWidget();

	QGridLayout* grid = new QGridLayout;
	grid->addWidget(widget1, 0, 0);
	//      grid->addWidget( widget2, 0, 1 );
	//      grid->addWidget( widget3, 1, 0 );
	//      grid->addWidget( widget4, 1, 1 );
	setLayout(grid);

	connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
	_timer.start(10);
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