#include "ViewerWidget.h"

osg::AnimationPath* createAnimationPath(const osg::Vec3& center, const osg::Vec3& scale, float radius, double looptime)
{
	// set up the animation path
	osg::AnimationPath* animationPath = new osg::AnimationPath;
	animationPath->setLoopMode(osg::AnimationPath::LOOP);

	int numSamples = 40;
	float yaw = 0.0f;
	float yaw_delta = 2.0f*osg::PI / ((float)numSamples - 1.0f);
	float roll = osg::inDegrees(0.0f/*30.0f*/);

	double time = 0.0f;
	double time_delta = looptime / (double)numSamples;
	for (int i = 0; i < numSamples; ++i)
	{
		osg::Vec3 position(center + osg::Vec3(sinf(yaw)*radius, cosf(yaw)*radius, 0.0f));
		osg::Quat rotation(osg::Quat(roll, osg::Vec3(0.0, 1.0, 0.0))*osg::Quat(-(yaw + osg::inDegrees(90.0f)), osg::Vec3(0.0, 0.0, 1.0)));

		animationPath->insert(time, osg::AnimationPath::ControlPoint(position, rotation, scale));

		yaw += yaw_delta;
		time += time_delta;

	}
	return animationPath;
}

osg::Node* createBase(const osg::Vec3& center, float radius)
{
	int numTilesX = 10;
	int numTilesY = 10;

	float width = 2 * radius;
	float height = 2 * radius;

	osg::Vec3 v000(center - osg::Vec3(width*0.5f, height*0.5f, 0.0f));
	osg::Vec3 dx(osg::Vec3(width / ((float)numTilesX), 0.0, 0.0f));
	osg::Vec3 dy(osg::Vec3(0.0f, height / ((float)numTilesY), 0.0f));

	// fill in vertices for grid, note numTilesX+1 * numTilesY+1...
	osg::Vec3Array* coords = new osg::Vec3Array;
	int iy;
	for (iy = 0; iy <= numTilesY; ++iy)
	{
		for (int ix = 0; ix <= numTilesX; ++ix)
		{
			coords->push_back(v000 + dx*(float)ix + dy*(float)iy);
		}
	}

	//Just two colours - black and white.
	osg::Vec4Array* colors = new osg::Vec4Array;
	colors->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f)); // white
	colors->push_back(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f)); // black

	osg::ref_ptr<osg::DrawElementsUShort> whitePrimitives = new osg::DrawElementsUShort(GL_QUADS);
	osg::ref_ptr<osg::DrawElementsUShort> blackPrimitives = new osg::DrawElementsUShort(GL_QUADS);

	int numIndicesPerRow = numTilesX + 1;
	for (iy = 0; iy < numTilesY; ++iy)
	{
		for (int ix = 0; ix < numTilesX; ++ix)
		{
			osg::DrawElementsUShort* primitives = ((iy + ix) % 2 == 0) ? whitePrimitives.get() : blackPrimitives.get();
			primitives->push_back(ix + (iy + 1)*numIndicesPerRow);
			primitives->push_back(ix + iy*numIndicesPerRow);
			primitives->push_back((ix + 1) + iy*numIndicesPerRow);
			primitives->push_back((ix + 1) + (iy + 1)*numIndicesPerRow);
		}
	}

	// set up a single normal
	osg::Vec3Array* normals = new osg::Vec3Array;
	normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));

	osg::Geometry* geom = new osg::Geometry;
	geom->setVertexArray(coords);

	geom->setColorArray(colors, osg::Array::BIND_PER_PRIMITIVE_SET);

	geom->setNormalArray(normals, osg::Array::BIND_OVERALL);

	geom->addPrimitiveSet(whitePrimitives.get());
	geom->addPrimitiveSet(blackPrimitives.get());

	osg::Geode* geode = new osg::Geode;
	geode->addDrawable(geom);

	return geode;
}

osg::Node* createMovingModel(const osg::Vec3& center, float radius)
{
	float animationLength = 10.0f;

	osg::AnimationPath* animationPath = createAnimationPath(center, osg::Vec3(), radius, animationLength);

	osg::Group* model = new osg::Group;

	osg::Node* glider = osgDB::readNodeFile("glider.osgt");
	if (glider)
	{
		const osg::BoundingSphere& bs = glider->getBound();

		float size = radius / bs.radius()*0.3f;
		osg::MatrixTransform* positioned = new osg::MatrixTransform;
		positioned->setDataVariance(osg::Object::STATIC);
		positioned->setMatrix(osg::Matrix::translate(-bs.center())*
			osg::Matrix::scale(size, size, size)*
			osg::Matrix::rotate(osg::inDegrees(-90.0f), 0.0f, 0.0f, 1.0f));

		positioned->addChild(glider);

		osg::PositionAttitudeTransform* xform = new osg::PositionAttitudeTransform;
		xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath, 0.0, 1.0));
		xform->addChild(positioned);

		model->addChild(xform);
	}

	osg::Node* cessna = osgDB::readNodeFile("cessna.osgt");
	if (cessna)
	{
		const osg::BoundingSphere& bs = cessna->getBound();

		float size = radius / bs.radius()*0.3f;
		osg::MatrixTransform* positioned = new osg::MatrixTransform;
		positioned->setDataVariance(osg::Object::STATIC);
		positioned->setMatrix(osg::Matrix::translate(-bs.center())*
			osg::Matrix::scale(size, size, size)*
			osg::Matrix::rotate(osg::inDegrees(180.0f), 0.0f, 0.0f, 1.0f));

		positioned->addChild(cessna);

		osg::MatrixTransform* xform = new osg::MatrixTransform;
		xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath, 0.0f, 2.0));
		xform->addChild(positioned);

		model->addChild(xform);
	}

	return model;
}

osg::Node* createModel(bool overlay, osgSim::OverlayNode::OverlayTechnique technique)
{
	osg::Vec3 center(0.0f, 0.0f, 0.0f);
	float radius = 3.0f;

	osg::Group* root = new osg::Group;

	float baseHeight = center.z() - radius*0.5;
	osg::Node* baseModel = createBase(osg::Vec3(center.x(), center.y(), baseHeight), radius);
	//osg::Node* baseModel = osgDB::readNodeFile("D:/osg3.2.0/taiwan/iso.ive");
	osg::Node* movingModel = createMovingModel(center, radius*0.8f);

	if (overlay)
	{
		osgSim::OverlayNode* overlayNode = new osgSim::OverlayNode(technique);
		overlayNode->setContinuousUpdate(true);
		overlayNode->setOverlaySubgraph(movingModel);
		overlayNode->setOverlayBaseHeight(baseHeight - 0.01);
		overlayNode->addChild(baseModel);
		root->addChild(overlayNode);
	}
	else
	{

		root->addChild(baseModel);
	}

	root->addChild(movingModel);

	return root;
}

ViewerWidget::ViewerWidget(osgViewer::ViewerBase::ThreadingModel threadingModel) : QWidget()
{
	setThreadingModel(threadingModel);

	// disable the default setting of viewer.done() by pressing Escape.
	setKeyEventSetsDone(0);

	osg::Group* _root = new osg::Group;
	//osg::Node* cessna = osgDB::readNodeFile("glider.osg"/*"su27.IVE"*/); 
	osg::Node* cessna = osgDB::readNodeFile("c:/a/zz.FBX"/*"su27.IVE"*/);

	//cessna->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

	osg::MatrixTransform* pLocalRotate = new osg::MatrixTransform;
	//pLocalRotate->setMatrix(osg::Matrix::rotate(osg::inDegrees(-90.0f), 0.0f, 0.0f, 1.0f));
	pLocalRotate->addChild(cessna);

	osg::Matrix matrixRotate;
	matrixRotate.makeRotate(osg::inDegrees(-90.0f), osg::Vec3f(1.0, 0.0, 0.0));

	osg::Matrix matrixRotate2;
	matrixRotate2.makeRotate(osg::inDegrees(180.0f), osg::Vec3f(0.0, 1.0, 0.0));

	pLocalRotate->postMult(matrixRotate);
	pLocalRotate->postMult(matrixRotate2);

	//创建位置变换节点pat1,缩放
	osg::PositionAttitudeTransform* pat1 = new osg::PositionAttitudeTransform;

	pat1->setPosition(osg::Vec3(121, 22.0f, 0.1f));
	//pat1->setScale(osg::Vec3(0.01f, 0.01f, 0.01f));

	pat1->addChild(pLocalRotate);

	osg::Vec3 center(121.038, 23.613, 0.1f);
	osg::Vec3 scale(0.5f, 0.5f, 0.5f);

	float radius = 1;
	float animationLength = 20.0f;
	osg::AnimationPath* animationPath = createAnimationPath(center, scale, radius, animationLength);
	pat1->setUpdateCallback(new osg::AnimationPathCallback(animationPath, 0.0, 1.0));

	osg::Node* pNode = osgDB::readNodeFile("D:/osg3.2.0/taiwan/iso.ive");
	_root->addChild(pNode);
	_root->addChild(pat1);

	// 		bool overlay = false;
	// 		osgSim::OverlayNode::OverlayTechnique technique = osgSim::OverlayNode::OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY;
	// 
	// 		// load the nodes from the commandline arguments.
	// 		osg::Node* model = createModel(overlay, technique);
	// 		if (!model)
	// 		{
	// 			return;
	// 		}
	// 
	// 		// tilt the scene so the default eye position is looking down on the model.
	// 		osg::MatrixTransform* rootnode = new osg::MatrixTransform;
	// 		rootnode->setMatrix(osg::Matrix::rotate(osg::inDegrees(30.0f), 1.0f, 0.0f, 0.0f));
	// 		rootnode->addChild(model);
	// 
	// 		// run optimization over the scene graph
	// 		osgUtil::Optimizer optimzer;
	// 		optimzer.optimize(rootnode);

	//QWidget* widget1 = addViewWidget(createGraphicsWindow(0, 0, 100, 100), /*rootnode*/_root, pNode);

	osgQt::GraphicsWindowQt* gw = createGraphicsWindow(0, 0, 100, 100);
	osgViewer::View* view = new osgViewer::View;
	addView(view);

	osg::Camera* camera = view->getCamera();
	camera->setGraphicsContext(gw);

	const osg::GraphicsContext::Traits* traits = gw->getTraits();

	camera->setClearColor(osg::Vec4(0.2, 0.2, 0.6, 1.0));
	camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
	camera->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(traits->width) / static_cast<double>(traits->height), 1.0f, 10000.0f);

	view->setSceneData(_root);
	view->addEventHandler(new osgViewer::StatsHandler);
	osgGA::TerrainManipulator* pManipulator = new osgGA::TerrainManipulator;
	pManipulator->setNode(pNode);
	// 		osgGA::NodeTrackerManipulator* pManipulator = new osgGA::NodeTrackerManipulator;
	// 		pManipulator->setTrackNode(pat1);
	view->setCameraManipulator(pManipulator/*new osgGA::TrackballManipulator*/);

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