#include "ViewerWidget.h"
#include "DataManager.h"
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osg/CullSettings>

#include <osgOcean/OceanScene>
#include <osgOcean/Version>
#include <osgOcean/ShaderManager>
#include <osg/ComputeBoundsVisitor>
#include <osg/Depth>
#include <gps_rcs_files_read.h>
#include "AutoSwitchMatrixManipulator.h"

#include "Scene.h"

osgViewer::View* g_pView = nullptr;

class MyNodeTrackerManipulator : public osgGA::NodeTrackerManipulator
{
public:

	MyNodeTrackerManipulator(){}
	~MyNodeTrackerManipulator(){}

protected:

	void computeHomePosition(const osg::Camera *camera, bool useBoundingBox) override
	{
		if (getNode())
		{
			osg::BoundingSphere boundingSphere;
			boundingSphere = getNode()->getBound();

			// set dist to default
			double dist = 3.5f * boundingSphere.radius();

			if (camera)
			{
				// try to compute dist from frustum
				double left, right, bottom, top, zNear, zFar;
				if (camera->getProjectionMatrixAsFrustum(left, right, bottom, top, zNear, zFar))
				{
					double vertical2 = fabs(right - left) / zNear / 2.;
					double horizontal2 = fabs(top - bottom) / zNear / 2.;
					double dim = horizontal2 < vertical2 ? horizontal2 : vertical2;
					double viewAngle = atan2(dim, 1.);
					dist = boundingSphere.radius() / sin(viewAngle);
				}
				else
				{
					// try to compute dist from ortho
					if (camera->getProjectionMatrixAsOrtho(left, right, bottom, top, zNear, zFar))
					{
						dist = fabs(zFar - zNear) / 2.;
					}
				}
			}

			// set home position
			setHomePosition(boundingSphere.center() + osg::Vec3d(0.0, /*dist * 0.01*/0.03, 0.0f),
				boundingSphere.center(),
				osg::Vec3d(0.0f, 0.0f, 1.0f),
				_autoComputeHomePosition);
		}
	}
};

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

osg::Node* createLightSource(unsigned int num)
{
	osg::ref_ptr<osg::Light> light = new osg::Light;
	light->setLightNum(num);
//	light->setDirection(vecDir);
	light->setAmbient(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
	//设置散射光的颜色
	light->setDiffuse(osg::Vec4(0.8f,0.8f,0.8f,1.0f));
	 
	light->setSpecular(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
	light->setPosition( osg::Vec4(0.0f, 0.0f, 1.0f, 0.0f) );

	osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource;
	lightSource->setLight(light);

	return lightSource.release();
}

void SetTerrainManipulator()
{
	DataManager* pManager = DataManager::Instance();
	osgGA::TerrainManipulator* pManipulator = new osgGA::TerrainManipulator;
	pManipulator->setNode(pManager->GetTerrainNode());
	g_pView->setCameraManipulator(pManipulator);
}

void SetNodeTrackerManipulator(int nNodeIndex = 0)
{
	DataManager* pManager = DataManager::Instance();
	//osgGA::NodeTrackerManipulator* pManipulator = new osgGA::NodeTrackerManipulator;
	MyNodeTrackerManipulator* pManipulator = new MyNodeTrackerManipulator;

	osg::Node* pNode = nNodeIndex == 0 ? pManager->GetAerocraftNode() : pManager->GetTargetObjectNode();
	pManipulator->setTrackNode(pNode);

	g_pView->setCameraManipulator(pManipulator/*new osgGA::TrackballManipulator*/);
	pManipulator->setMinimumDistance(0.002);
}

void SetAutoManipulator()
{
	double dLeft, dTop, dRight, dBottom, dH;
	bool bTag = false;
	extern QVector<dataunit>  g_vecData;
	for (auto& dataUnit : g_vecData)
	{
		if (!bTag)
		{
			bTag = !bTag;
			dLeft = dataUnit.plane_lon;
			dRight = dataUnit.plane_lon;
			dTop = dataUnit.plane_lat;
			dBottom = dataUnit.plane_lat;
			dH = dataUnit.plane_Height;
		}
		else
		{
			if (dLeft > dataUnit.plane_lon)
			{
				dLeft = dataUnit.plane_lon;
			}

			if (dRight < dataUnit.plane_lon)
			{
				dRight = dataUnit.plane_lon;
			}

			if (dTop < dataUnit.plane_lat)
			{
				dTop = dataUnit.plane_lat;
			}

			if (dBottom > dataUnit.plane_lat)
			{
				dBottom = dataUnit.plane_lat;
			}
		}
	}

	int nSize = g_vecData.size();
	double dTime = g_vecData[0].dTime;

	osg::Vec3d position((dRight + dLeft) * 0.5, dBottom - (dTop - dBottom) , dH * 0.00001141 * 0.5);
	osg::Vec3d position2 = position;
	position2.z() = dH * 0.00001141 + (dRight - dLeft) * 1.5;

	osg::Vec3d position3((dRight + dLeft) * 0.5, (dBottom + dTop) * 0.5, position2.z());

	osg::AnimationPath* animationPath = new osg::AnimationPath;
	animationPath->setLoopMode(osg::AnimationPath::LOOP);

	osg::Quat quat;
	quat.makeRotate(osg::Vec3d(0.0, 0.0, -1.0), osg::Vec3d(0.0, 1.0, 0.0));
	animationPath->insert(0.0, osg::AnimationPath::ControlPoint(position, quat));
	animationPath->insert(3.0, osg::AnimationPath::ControlPoint(position, quat));

	osg::Quat quat2;
	quat2.makeRotate(osg::Vec3d(0.0, 0.0, -1.0), osg::Vec3d(0.0, 1.0, -1.6));
	animationPath->insert(4.5, osg::AnimationPath::ControlPoint(position2, quat2));
	animationPath->insert(6.0, osg::AnimationPath::ControlPoint(position3));
	animationPath->insert(9.0, osg::AnimationPath::ControlPoint(position3));

	osgGA::AnimationPathManipulator *animationPathMp = new osgGA::AnimationPathManipulator();
	animationPathMp->setAnimationPath(animationPath);

	osgGA::AutoSwitchMatrixManipulator* pAutoManipulator = new osgGA::AutoSwitchMatrixManipulator;

	DataManager* pManager = DataManager::Instance();
	osgGA::NodeTrackerManipulator* pTrackTarget = new MyNodeTrackerManipulator;
	pTrackTarget->setTrackNode(pManager->GetTargetObjectNode());

	osgGA::NodeTrackerManipulator* pTrackPlane = new MyNodeTrackerManipulator;
	pTrackPlane->setTrackNode(pManager->GetAerocraftNode());

	pAutoManipulator->AddManipulator(pTrackPlane);
	pAutoManipulator->AddManipulator(pTrackTarget);
	pAutoManipulator->AddManipulator(animationPathMp);

	g_pView->setCameraManipulator(pAutoManipulator);
}

ViewerWidget::ViewerWidget(osgViewer::ViewerBase::ThreadingModel threadingModel) : QWidget()
{
	setMinimumSize(QSize(400, 300));
	setThreadingModel(threadingModel);

	// disable the default setting of viewer.done() by pressing Escape.
	setKeyEventSetsDone(0);

	osgQt::GraphicsWindowQt* gw = createGraphicsWindow(0, 0, 100, 100);
	osgViewer::View* view = new osgViewer::View;
	view->setLightingMode(osg::View::HEADLIGHT/*SKY_LIGHT*/);

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
	pManager->LoadTargetObject("c:/102202.FBX"/*"c:/a/boat.FBX"*/);
	pManager->LoadAerocraft("c:/a/plane.FBX");

	//添加多光源
	if (0)
	{
		osg::Group* pRoot = dynamic_cast<osg::Group*>(pManager->GetRootNode());
		osg::ref_ptr<osg::StateSet> stateset = pRoot->getOrCreateStateSet();
		stateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
		stateset->setMode(GL_LIGHT6, osg::StateAttribute::ON);	// GL_LIGHT0是默认光源
		// 设置6个光源 解决光照问题
		osg::Vec3d ptLight;
		osg::Vec3d ptCenter = osg::Vec3d(0, 0, 0);
		double dDis = 20.0;
		{
			ptLight = ptCenter + osg::Z_AXIS * dDis;
			osg::Node *pNodeLight = createLightSource(6);
			pNodeLight->setName("light6");
			pRoot->addChild(pNodeLight);
		}
	}

	//天空盒
	if (0)
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

	view->getLight()->setPosition(osg::Vec4(0.0f, 0.0f, 1.0f, 0.0f));
	// 环境光
	view->getLight()->setAmbient(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	// 漫反射光
	view->getLight()->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

	SetNodeTrackerManipulator();
	//SetTerrainManipulator();
	//SetAutoManipulator();

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