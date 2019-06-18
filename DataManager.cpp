#include "DataManager.h"
#include "videoplayer.h"
#include "OrientationCallback.h"

#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/LineWidth>
#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osg/BlendFunc>
#include <osg/ShapeDrawable>
#include <osg/PolygonMode>
#include <osg/Depth>
#include <osg/LOD>
#include <QtGui/QVector3D>
#include <QtCore/QFile>
#include <QtCore/QVector>
#include <QtCore/QTime>
#include <QtCore/QMap>
#include "gps_rcs_files_read.h"
#include "samplingthread.h"
#include "PlaneLOD.h"
#include <osgOcean/ShaderManager>
#include <osgOcean/FFTOceanSurface>
#include <osg/PolygonOffset>
#include <osg/Switch>
#include "plot.h"
#include "signaldata.h"

SamplingThread* g_pSampleThread = nullptr;
Plot* g_pPlot = nullptr;
VideoPlayer* g_pVideoPlayer = nullptr;
DataManager* g_DataManager = nullptr;

QVector<RCSRecord> g_vecRCSRecord;
QMutex g_MutexData;

osgOcean::FFTOceanSurface* g_surface = nullptr;

void SetAutoManipulator(double dLeft, double dTop, double dRight, double dBottom, double dH);

void AddOcean(double dLon, double dLat, osg::Group* pParent)
{
	osgOcean::ShaderManager::instance().enableShaders(true);
	// 		osg::ref_ptr<osgOcean::FFTOceanSurface> surface = new osgOcean::FFTOceanSurface(64, 256, 17
	// 			, osg::Vec2(1.1f, 1.1f), 12, 10, 0.8, 1e-8, true, 2.5, 20.0, 256);

	// 		osg::ref_ptr<osgOcean::FFTOceanSurface> surface = new osgOcean::FFTOceanSurface(/*64, 256, 17
	// 			, osg::Vec2(1.1f, 1.1f), 12, 10, 0.8, 1e-8, true, 2.5, 1.0, 32*/);

	if (g_surface == nullptr)
	{
		g_surface = new osgOcean::FFTOceanSurface(32);

		g_surface->setWaveScaleFactor(5e-9f * 0.25);
		// 		surface->setFoamBottomHeight(2.2);
		// 		surface->setFoamTopHeight(3.0);
		// 
		g_surface->enableCrestFoam(true);

		g_surface->setLightColor(osg::Vec4f(0.0, 0.0, 1.0, 1.0));
	}

	osg::PositionAttitudeTransform* pTransform = new osg::PositionAttitudeTransform;
	pTransform->addChild(g_surface);

	pTransform->setScale(osg::Vec3d(0.00032, 0.00032, 0.00032));
	pTransform->setPosition(osg::Vec3d(dLon, dLat, -0.0004/*-0.004*/));

	//可能缩放变换会造成光照结果过于明亮或暗淡，要在StateSet中允许法线的重缩放模式。
	osg::StateSet* pStateSet = pTransform->getOrCreateStateSet();
	pStateSet->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::OVERRIDE);
	
	//osg::Depth* depth = new osg::Depth;
	//depth->setFunction(osg::Depth::ALWAYS);
	//depth->setRange(0.1, 1.0);
	//pStateSet->setAttributeAndModes(depth, osg::StateAttribute::ON);
	//pStateSet->setRenderBinDetails(-2, "RenderBin");

	osg::LOD* pLOD = new osg::LOD;
	pLOD->addChild(pTransform, 0, 0.8);
	pParent->addChild(pLOD);
}

osg::AnimationPath* createLineAnimationPath(const osg::Vec3d& startPoint
	, const osg::Vec3d& endPoint, const osg::Vec3d& scale, double time)
{
	osg::AnimationPath* animationPath = new osg::AnimationPath;
	animationPath->setLoopMode(osg::AnimationPath::LOOP);

	animationPath->insert(0.0, osg::AnimationPath::ControlPoint(startPoint, osg::Quat(), scale));
	animationPath->insert(time, osg::AnimationPath::ControlPoint(endPoint, osg::Quat(), scale));

	return animationPath;
}

osg::AnimationPath* createCircleAnimationPath(const osg::Vec3d& center, const osg::Vec3d& scale, double radius, double looptime)
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
		osg::Vec3d position(center + osg::Vec3d(sinf(yaw)*radius, cosf(yaw)*radius, 0.0f));
		osg::Quat rotation(osg::Quat(-(yaw), osg::Vec3d(0.0, 0.0, 1.0)));
		animationPath->insert(time, osg::AnimationPath::ControlPoint(position, /*osg::Quat()*/rotation, scale));

		yaw += yaw_delta;
		time += time_delta;

	}
	return animationPath;
}

osg::AnimationPath* createPlaneAnimationPath(const QString& strPlaneGPS, const osg::Vec3d& scale, double looptime)
{
	QFile file(strPlaneGPS);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return 0;

	QVector<double> vecTime;
	QVector<double> vecLon;
	QVector<double> vecLat;
	QVector<double> vecH;

	bool bIni = false;
	QTime timeStart;

	int nCount = 0;
	while (!file.atEnd()) {
		QByteArray line = file.readLine();

		nCount = nCount % 100;
		if (nCount == 0)
		{
			QString strLine = line;
			QStringList strList = strLine.split(QRegExp("\\s+"));

			vecLon.push_back(strList[2].toDouble());
			vecLat.push_back(strList[3].toDouble());
			vecH.push_back(strList[4].toDouble() / 111000.0);
		}

		nCount++;
	}

	// set up the animation path
	osg::AnimationPath* animationPath = new osg::AnimationPath;
	animationPath->setLoopMode(osg::AnimationPath::LOOP);

	int nSize = vecLon.size();
	double dTimeStep = looptime / nSize;

	double dTime = 0.0;
	for (int i = 0; i < nSize; i++)
	{
		osg::Quat quat;
		osg::Vec3d position(vecLon[i], vecLat[i], vecH[i]);

		if (i != 0 && i < nSize - 1)
		{
			osg::Vec3 vec0(1.0, 0.0, 0.0);
			osg::Vec3 vec1(vecLon[i + 1] - vecLon[i - 1], vecLat[i + 1] - vecLat[i - 1], 0.0);

			quat.makeRotate(vec0, vec1);
		}

		animationPath->insert(dTime, osg::AnimationPath::ControlPoint(position, quat, scale));

		dTime += dTimeStep;
	}

	return animationPath;
}

osg::AnimationPath* createTargetAnimationPath(const QString& strTargetGPS, const osg::Vec3d& scale, double looptime)
{
	QFile file(strTargetGPS);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return 0;

	QVector<QPair<double, double>> vecPos;

	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		QString strLine = line;

		QStringList strList = strLine.split('#');
		double dLat = strList[0].toDouble() + strList[1].toDouble() / 60.0;
		double dLon = strList[2].toDouble() + strList[3].toDouble() / 60.0;

		vecPos.push_back(qMakePair(dLon, dLat));
	}

	// set up the animation path
	osg::AnimationPath* animationPath = new osg::AnimationPath;
	animationPath->setLoopMode(osg::AnimationPath::LOOP);

	int nSize = vecPos.size();
	double dTimeStep = looptime / nSize;

	double dTime = 0.0;
	for (int i = 0; i < nSize; i++)
	{
		osg::Quat quat;
		osg::Vec3d position(vecPos[i].first, vecPos[i].second, -0.0040);

		if (i != 0 && i < nSize - 1)
		{
			osg::Vec3d position0(vecPos[i - 1].first, vecPos[i - 1].second, -0.004);
			osg::Vec3d position1(vecPos[i + 1].first, vecPos[i + 1].second, -0.004);

			osg::Vec3 vec0(1.0, 0.0, 0.0);
			osg::Vec3 vec1 = position1 - position0;

			quat.makeRotate(vec0, vec1);
		}

		animationPath->insert(dTime, osg::AnimationPath::ControlPoint(position, quat, scale));
		dTime += dTimeStep;
	}

	return animationPath;
}

DataManager::DataManager()
{
	m_pRoot = new osg::Group;
	osg::StateSet* pStateSet = m_pRoot->getOrCreateStateSet();
	//pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	m_pAerocraftNode = nullptr;
	m_pTargetNode = nullptr;
	m_pTerrainNode = nullptr;

	m_dRotateX = -90.0f;
	m_dRotateY = 180.0f;
	m_dRotateZ = 270.0f;

	m_circleCenter = osg::Vec3d(121.038, 23.613, 0.1);
	m_dScale = 0.0005;
	m_dCircleRadius = 0.8;
	m_dCircleTime = 400.0;

	m_lineStartPoint = osg::Vec3d(120.038, 23.613, 0.1);
	m_lineEndPoint = osg::Vec3d(122.038, 23.613, 0.1);
	m_dLineTime = 200.0;

	m_pAerocraftLocalMatrixNode = new osg::MatrixTransform;
	m_ePlanePathType = PathType_Circle;

	SetAerocraftRotate(m_dRotateX, m_dRotateY, m_dRotateZ);

	//创建位置变换节点pat1,缩放
	m_pAerocraftAnimationNode = new osg::PositionAttitudeTransform;

	m_pAerocraftAnimationNode->setPosition(osg::Vec3d(121, 22.0, 0.1));
	m_pAerocraftAnimationNode->setScale(osg::Vec3(m_dScale, m_dScale, m_dScale));

	m_pAerocraftAnimationNode->addChild(m_pAerocraftLocalMatrixNode);

	osg::Vec3d scale(m_dScale, m_dScale, m_dScale);

	m_pRoot->addChild(m_pAerocraftAnimationNode);

	m_dTargetRotateX = -90.0;
	m_dTargetRotateY = 180.0;
	m_dTargetRotateZ = 270.0;
	m_dTargetScale = 0.00001;

	m_pTargetLocalMatrixNode = new osg::MatrixTransform;
	m_pTargetAnimationNode = new osg::PositionAttitudeTransform;
	m_pTargetAnimationNode->addChild(m_pTargetLocalMatrixNode);
	m_pRoot->addChild(m_pTargetAnimationNode);

	osg::Vec3d scaleTarget(m_dTargetScale, m_dTargetScale, m_dTargetScale);

	m_vTargetPos = osg::Vec3d(121.338, 22.543, -0.0103);
	SetTargetPara(m_vTargetPos, m_dTargetRotateX, m_dTargetRotateY, m_dTargetRotateZ, m_dTargetScale);

	m_pRadarBeamNode = nullptr;

	//雷达波局部矩阵的位移和旋转
	m_dRadarOffsetX = 0.0;
	m_dRadarOffsetY = 0.0;
	m_dRadarOffsetZ = 0.0;
	m_dRadarRotateX = 0.0;
	m_dRadarRotateY = 0.0;
	m_dRadarRotateZ = 0.0;

	m_dRadarScale = 1.0;
	m_pRadarBeamLocalMatrixNode = nullptr;
	m_pAeroPathLine = new osg::Geode();

	osg::ref_ptr<osg::Vec3Array> vecarry1 = new osg::Vec3Array();
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();

	{
		vecarry1->push_back(osg::Vec3d(0, 0, 0));
		vecarry1->push_back(osg::Vec3d(1, 1, 1));
		vecarry1->push_back(osg::Vec3d(2, 2, 2));
		geometry->setVertexArray(vecarry1.get());

		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
		geometry->setColorArray(colors, osg::Array::BIND_OVERALL);

		geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 3));
	}
	m_pAeroPathLine->addDrawable(geometry.get());
	m_pRoot->addChild(m_pAeroPathLine);
	osg::StateSet* pStateSet1 = m_pAeroPathLine->getOrCreateStateSet();
	osg::LineWidth* pWidth = new osg::LineWidth(4.0);
	pStateSet1->setAttribute(pWidth);
}

DataManager::~DataManager()
{
}

void DataManager::GetPlanePathEnv(double& dx1, double& dy1, double& dx2, double& dy2, double& dH)
{
	dx1 = m_dLeft;
	dy1 = m_dBottom;

	dx2 = m_dRight;
	dy2 = m_dTop;
	dH = m_dH;
}

#include <QtGui/QPainter>

bool DataManager::LoadDataAndDisplay(QString gpsfile, QString targpsfile, QString rcsfile, QString video)
{
	cTime timeStart;
	QVector<dataunit> vecData;

	if (!gps_rcs_files_read(gpsfile, targpsfile, rcsfile, vecData, timeStart))
		return false;


	{
		QMutexLocker locker(&g_MutexData);
		g_vecRCSRecord.clear();
		g_vecRCSRecord.reserve(vecData.size());

		double dTimeFirst = vecData[0].dTime;

// 		QImage image(800, 600, QImage::Format_RGB32);
// 		image.fill(Qt::white);
// 		QPainter painter(&image);
// 		painter.setPen(Qt::red);
// 
// 		QPointF* pPoints = new QPointF[vecData.size()];
// 		int i = 0;

		for (auto& dataUint : vecData)
		{
			RCSRecord record;
			record.dTime = dataUint.dTime - dTimeFirst;
			record.angle = dataUint.angle;
			record.RCS_dB = dataUint.RCS_dB;

			g_vecRCSRecord.push_back(record);

// 			pPoints[i].setX(dataUint.angle * 2.0);
// 			pPoints[i].setY(dataUint.RCS_dB *8.0);
// 			i++;
		}

// 		painter.drawPolyline(pPoints, vecData.size());
// 		delete pPoints;
// 		pPoints = nullptr;
// 		image.save("d:/rcs_test.png");
	}

	int nCount = vecData.size();
	double dIncre = (vecData[nCount - 1].dTime - vecData[0].dTime) / (nCount - 1);

	for (int i = 0; i < nCount; i++)
	{
		vecData[i].dTime = vecData[0].dTime + dIncre * i;
	}

	int nTemp = 0;
	double dKey = /*1.8e-6*/0.00005;

	if (vecData.isEmpty())
		return false;

	m_dLeft = vecData[0].plane_lon;
	m_dRight = vecData[0].plane_lon;
	m_dTop = vecData[0].plane_lat;
	m_dBottom = vecData[0].plane_lat;
	m_dH = vecData[0].plane_Height;

	QVector<dataunit> listData;
	listData.push_back(vecData[0]);

	for (int i = 1; i < nCount; i++)
	{
		double dLon = vecData[i].plane_lon - vecData[nTemp].plane_lon;
		double dLat = vecData[i].plane_lat - vecData[nTemp].plane_lat;

		if (sqrt(dLon * dLon + dLat * dLat) > dKey)
		{
			nTemp = i;
			listData.push_back(vecData[i]);

			//求出飞行路线的外接范围。
			if (m_dLeft > vecData[i].plane_lon)
				m_dLeft = vecData[i].plane_lon;

			if (m_dRight < vecData[i].plane_lon)
				m_dRight = vecData[i].plane_lon;

			if (m_dTop < vecData[i].plane_lat)
				m_dTop = vecData[i].plane_lat;

			if (m_dBottom > vecData[i].plane_lat)
				m_dBottom = vecData[i].plane_lat;
		}
	}

	vecData.clear();
	if (listData.isEmpty())
		return false;

	osg::AnimationPath* animationPathPlane = new osg::AnimationPath;
	animationPathPlane->setLoopMode(osg::AnimationPath::LOOP);

	osg::AnimationPath* animationPathTarget = new osg::AnimationPath;
	animationPathTarget->setLoopMode(osg::AnimationPath::LOOP);

	m_pAerocraftAnimationNode->setUpdateCallback(new osg::AnimationPathCallback(animationPathPlane));
	m_pTargetAnimationNode->setUpdateCallback(new osg::AnimationPathCallback(animationPathTarget));

	osg::Vec3d scale(m_dScale, m_dScale, m_dScale);
	double dTime = listData[0].dTime;
	int nSize = listData.size();

	for (int i = 0; i < nSize; i++)
	{
		osg::Quat quat;
		osg::Vec3d position(listData[i].plane_lon, listData[i].plane_lat, listData[i].plane_Height * 0.000008983152841195214); // * 0.000008983152841195214 转为经纬度

		if (i != 0 && i < nSize - 1)
		{
			osg::Vec3 vec0(1.0, 0.0, 0.0);

			double dLon = listData[i + 1].plane_lon - listData[i - 1].plane_lon;
			double dLat = listData[i + 1].plane_lat - listData[i - 1].plane_lat;
			if (dLon == 0.0 && dLat == 0.0)
			{
				continue;
			}

			osg::Vec3 vec1(dLon, dLat, 0.0);
			quat.makeRotate(vec0, vec1);
		}

		animationPathPlane->insert(listData[i].dTime - dTime, osg::AnimationPath::ControlPoint(position, quat, scale));
	}

	osg::Vec3d scaleTarget(m_dTargetScale, m_dTargetScale, m_dTargetScale);
	for (int i = 0; i < nSize; i++)
	{
		osg::Quat quat;
		osg::Vec3d position(listData[i].target_lon, listData[i].target_lat, listData[i].target_Height *  0.000008983152841195214);

		if (i != 0 && i < nSize - 1)
		{
			osg::Vec3 vec0(1.0, 0.0, 0.0);

			double dLon = listData[i + 1].target_lon - listData[i - 1].target_lon;
			double dLat = listData[i + 1].target_lat - listData[i - 1].target_lat;

			if (dLon == 0.0 && dLat == 0.0)
			{
				continue;
			}

			osg::Vec3 vec1(dLon, dLat, 0.0);
		}

		animationPathTarget->insert(listData[i].dTime - dTime, osg::AnimationPath::ControlPoint(position, quat, scaleTarget));
	}

	listData.clear();

	SignalData::instance().Clear();

	//加载曲线显示
	if (g_pSampleThread)
	{
		if (g_pSampleThread->isRunning())
		{
			g_pSampleThread->Cancel();
			//g_pSampleThread->wait(1000);
		}

		delete g_pSampleThread;
		g_pSampleThread = nullptr;
	}

	g_pSampleThread = new SamplingThread;
	g_pSampleThread->setFrequency(0.05);
	g_pSampleThread->setAmplitude(40);
	g_pSampleThread->setInterval(10);
	g_pSampleThread->start();

	g_pPlot->start();
	g_pPlot->replot();

	SetAutoManipulator(m_dLeft, m_dTop, m_dRight, m_dBottom, m_dH);

	ClearPlanePathLine();
	ResetAnimationPath();

	if (g_pVideoPlayer)
		g_pVideoPlayer->setFileAndPlay(video);

	return true;
}

DataManager* DataManager::Instance()
{
	if (g_DataManager == nullptr)
	{
		g_DataManager = new DataManager;
	}

	return g_DataManager;
}

std::string Path(QString str)
{
	QString strDir = QCoreApplication::applicationDirPath();
	strDir += "/../data/" + str;

	static std::string s_str;

	QByteArray byteArray = strDir.toLocal8Bit();
	s_str = byteArray.data();

	return s_str;
}

void DataManager::LoadTerrain()
{
	if (m_pTerrainNode == nullptr)
	{
		osg::Group* pTerrainGroup = new osg::Group;
		m_pTerrainNode = pTerrainGroup;
		m_pRoot->addChild(m_pTerrainNode);

		//osg::Node* pNodeChina = osgDB::readNodeFile("d:/ive_google/china/china.ive");
		osg::Node* pNodeChina = osgDB::readNodeFile(Path("china/china.ive"));

		{
			osg::StateSet * pStateSet = pNodeChina->getOrCreateStateSet();
			pStateSet->setRenderBinDetails(-3, "RenderBin");
		}

		osg::MatrixTransform* pChinaTransform = new osg::MatrixTransform;
		pChinaTransform->setMatrix(osg::Matrix::translate(osg::Vec3d(0.0, 0.0, -0.0025)));
		pChinaTransform->addChild(pNodeChina);
		pTerrainGroup->addChild(pChinaTransform);

		//osg::Depth* depth = new osg::Depth;
		//depth->setFunction(osg::Depth::ALWAYS);
		//depth->setRange(1.0, 1.0);
		//osg::StateSet * pStateSet = pNode->getOrCreateStateSet();
		//pStateSet->setAttributeAndModes(depth, osg::StateAttribute::ON);
		//pStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
		//pStateSet->setRenderBinDetails(-2, "RenderBin");
		
		osg::Node* pNode = nullptr;
		//pNode = osgDB::readNodeFile("D:/ive_google/sanya16.ive");
		pNode = osgDB::readNodeFile(Path("sanya16.ive"));
		osg::StateSet * ss = pNode/*pTerrainGroup*/->getOrCreateStateSet();
		pTerrainGroup->addChild(pNode);		AddOcean(110.112462424978, 18.0959936703065, pTerrainGroup);
		if (1)
		{
			osg::Program * program = new osg::Program;
			program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShader));
			program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShader));

			osg::ref_ptr<osg::Uniform> sampler0 = new osg::Uniform("sampler0", 0);
			ss->addUniform(sampler0.get());

			ss->setAttributeAndModes(program, osg::StateAttribute::OVERRIDE);
			ss->setMode(GL_BLEND, osg::StateAttribute::ON);
			ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		}

		//pTerrainGroup->addChild(osgDB::readNodeFile("D:/rcsmodel/west.ive"));
		//pTerrainGroup->addChild(osgDB::readNodeFile("D:/rcsmodel/east_little.ive"));
		
		//pNode = osgDB::readNodeFile("D:/ive_google/huludao16.ive");
		pNode = osgDB::readNodeFile(Path("huludao16.ive"));
		pNode->setStateSet(ss);
		pTerrainGroup->addChild(pNode);
		AddOcean(121.061215478138, 40.6654974465535, pTerrainGroup);

		//pNode = osgDB::readNodeFile("D:/ive_google/zhoushan16.ive");
		pNode = osgDB::readNodeFile(Path("zhoushan16.ive"));
		pNode->setStateSet(ss);
		pTerrainGroup->addChild(pNode);
		AddOcean(122.730963656668, 29.829958362075, pTerrainGroup);

		//pNode = osgDB::readNodeFile("D:/ive_google/qinhuangdao16.ive");
		pNode = osgDB::readNodeFile(Path("qinhuangdao16.ive"));
		pNode->setStateSet(ss);
		pTerrainGroup->addChild(pNode);
		AddOcean(119.6827079830245, 39.8695409097925, pTerrainGroup);

		//pNode = osgDB::readNodeFile("D:/ive_google/dalianchanghai16.ive");
		pNode = osgDB::readNodeFile(Path("dalianchanghai16.ive"));
		pNode->setStateSet(ss);
		pTerrainGroup->addChild(pNode);
		AddOcean(122.8398126232475, 38.837338818795, pTerrainGroup);

		//pNode = osgDB::readNodeFile("D:/rcsmodel/dunhuang.ive");
		pNode = osgDB::readNodeFile(Path("dunhuang.ive"));
		osg::StateSet* stateset = pNode->getOrCreateStateSet();
		//osg::Depth* depth = new osg::Depth;
		//depth->setFunction(osg::Depth::ALWAYS);
		//depth->setRange(0.0, 0.9);

		//stateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
		//stateset->setRenderBinDetails(-1, "RenderBin");

		//stateset->setAttributeAndModes(new osg::PolygonOffset(-1.0f, -1.0f), osg::StateAttribute::ON);
		// 		stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
		// 		stateset->setRenderBinDetails(11, "RenderBin");
		pTerrainGroup->addChild(pNode);

		//pNode = osgDB::readNodeFile("D:/rcsmodel/dunhuang(mubiaoqu).ive");
		pNode = osgDB::readNodeFile(Path("dunhuang(mubiaoqu).ive"));
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		//pNode = osgDB::readNodeFile("D:/rcsmodel/dunhuang3.ive");
		pNode = osgDB::readNodeFile(Path("dunhuang3.ive"));
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		//pNode = osgDB::readNodeFile("D:/rcsmodel/hami(jichangqu).ive");
		pNode = osgDB::readNodeFile(Path("hami(jichangqu).ive"));
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		//pNode = osgDB::readNodeFile("D:/rcsmodel/hami(mubiaoqu).ive");
		pNode = osgDB::readNodeFile(Path("hami(mubiaoqu).ive"));
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		//pNode = osgDB::readNodeFile("D:/rcsmodel/jinzhou.ive");
		pNode = osgDB::readNodeFile(Path("jinzhou.ive"));
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		//pNode = osgDB::readNodeFile("D:/rcsmodel_new/kuerle.ive");
		pNode = osgDB::readNodeFile(Path("kuerle.ive"));
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		//pNode = osgDB::readNodeFile("D:/rcsmodel/liaoningchaoyang.ive");
		pNode = osgDB::readNodeFile(Path("liaoningchaoyang.ive"));
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		//pNode = osgDB::readNodeFile("D:/rcsmodel/qiemo.ive");
		pNode = osgDB::readNodeFile(Path("qiemo.ive"));
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		//pNode = osgDB::readNodeFile("D:/rcsmodel/xilinhaote.ive");
		pNode = osgDB::readNodeFile(Path("xilinhaote.ive"));
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);
	}
}

void DataManager::LoadAerocraft(const QString& strFile)
{
	m_pAerocraftNode = osgDB::readNodeFile(strFile.toLocal8Bit().data()/*"su27.IVE"*/);

	osgUtil::Optimizer optimzer;
	optimzer.optimize(m_pAerocraftNode);

	int nNumChild = m_pAerocraftLocalMatrixNode->getNumChildren();
	if (nNumChild > 0)
	{
		osg::Node* pNode = m_pAerocraftLocalMatrixNode->getChild(0);
		m_pAerocraftLocalMatrixNode->removeChild(pNode);
	}

	m_pAerocraftLocalMatrixNode->addChild(m_pAerocraftNode);

	LoadRadarBeam("");
}

void DataManager::SetAerocraftRotate(float x, float y, float z)
{
	m_dRotateX = x;
	m_dRotateY = y;
	m_dRotateZ = z;

	m_pAerocraftLocalMatrixNode->setMatrix(osg::Matrix());

	osg::Matrix matrixRotate;
	matrixRotate.makeRotate(osg::inDegrees(m_dRotateX), osg::Vec3f(1.0, 0.0, 0.0));

	osg::Matrix matrixRotate2;
	matrixRotate2.makeRotate(osg::inDegrees(m_dRotateY), osg::Vec3f(0.0, 1.0, 0.0));

	osg::Matrix matrixRotate3;
	matrixRotate3.makeRotate(osg::inDegrees(m_dRotateZ), osg::Vec3f(0.0, 0.0, 1.0));

	m_pAerocraftLocalMatrixNode->postMult(matrixRotate);
	m_pAerocraftLocalMatrixNode->postMult(matrixRotate2);
	m_pAerocraftLocalMatrixNode->postMult(matrixRotate3);
}

void DataManager::GetAerocraftRotate(float& x, float& y, float& z)
{
	x = m_dRotateX;
	y = m_dRotateY;
	z = m_dRotateZ;
}

void DataManager::SetAerocraftScale(double dScale)
{
	m_dScale = dScale;
	osg::Vec3d scale(m_dScale, m_dScale, m_dScale);
	m_pAerocraftAnimationNode->setScale(osg::Vec3(m_dScale, m_dScale, m_dScale));

	osg::Callback* pCallback = m_pAerocraftAnimationNode->getUpdateCallback();
	if (pCallback)
	{
		osg::AnimationPathCallback* pAnimationCallback = dynamic_cast<osg::AnimationPathCallback*>(pCallback);
		if (pAnimationCallback)
		{
			osg::AnimationPath* pAnimationPath = pAnimationCallback->getAnimationPath();
			osg::AnimationPath::TimeControlPointMap& pointMap = pAnimationPath->getTimeControlPointMap();

			for (osg::AnimationPath::TimeControlPointMap::iterator itr = pointMap.begin(); itr != pointMap.end(); itr++)
			{
				itr->second.setScale(osg::Vec3d(m_dScale, m_dScale, m_dScale));
			}
		}
	}

// 	osg::AnimationPath* animationPath = nullptr;
// 	if (m_ePlanePathType == PathType_Circle)
// 	{
// 		animationPath = createCircleAnimationPath(m_circleCenter, scale, m_dCircleRadius, m_dCircleTime);
// 	}
// 	else if (m_ePlanePathType == PathType_Line)
// 	{
// 		animationPath = createLineAnimationPath(m_lineStartPoint, m_lineEndPoint, scale, m_dLineTime);
// 	}
// 
// 	m_pAerocraftAnimationNode->setUpdateCallback(new osg::AnimationPathCallback(animationPath, 0.0, 1.0));
}

double DataManager::GetAerocraftScale()
{
	return m_dScale;
}

osg::Node* DataManager::GetRadarBeamNode()
{
	return m_pRadarBeamLocalMatrixNode;
	//return m_pRadarBeamNode;
}

osg::Node* DataManager::GetAerocraftNode()
{
	return m_pAerocraftNode;
}

osg::PositionAttitudeTransform* DataManager::GetPlaneParentNode()
{
	return m_pAerocraftAnimationNode;
}

osg::Node* DataManager::GetTargetObjectNode()
{
	return m_pTargetNode;
}

osg::Node* DataManager::GetTerrainNode()
{
	return m_pTerrainNode;
}

osg::Node* DataManager::GetRootNode()
{
	return m_pRoot;
}

void DataManager::SetCirclePathPara(const osg::Vec3d& vecCenter, double dRadius, double dTime)
{
	m_circleCenter = vecCenter;
	m_dCircleRadius = dRadius;
	m_dCircleTime = dTime;

	SetPathType(PathType_Circle);
}

void DataManager::GetCirclePathPara(osg::Vec3d& vecCenter, double& dRadius, double& dTime)
{
	vecCenter = m_circleCenter;
	dRadius = m_dCircleRadius;
	dTime = m_dCircleTime;
}

void DataManager::SetLinePathPara(const osg::Vec3d& vecStartPoint, const osg::Vec3d& vecEndPoint, double dTime)
{
	m_lineStartPoint = vecStartPoint;
	m_lineEndPoint = vecEndPoint;
	m_dLineTime = dTime;

	SetPathType(PathType_Line);
}

void DataManager::GetLinePathPara(osg::Vec3d& startPoint, osg::Vec3d& endPoint, double& dTime)
{
	startPoint = m_lineStartPoint;
	endPoint = m_lineEndPoint;
	dTime = m_dLineTime;
}

void DataManager::SetPathType(PathType type)
{
	m_ePlanePathType = type;
	osg::AnimationPath* animationPath = nullptr;
	osg::Vec3d scale(m_dScale, m_dScale, m_dScale);

	if (type == PathType_Circle)
	{
		animationPath = createCircleAnimationPath(m_circleCenter, scale, m_dCircleRadius, m_dCircleTime);
	}
	else if (type == PathType_Line)
	{
		animationPath = createLineAnimationPath(m_lineStartPoint, m_lineEndPoint, scale, m_dLineTime);
	}

	m_pAerocraftAnimationNode->setUpdateCallback(new osg::AnimationPathCallback(animationPath, 0.0, 1.0));
}

void DataManager::GetTargetPara(double& dLon, double& dLat, double& dHeight, double& dRotateX
	, double& dRotateY, double& dRotateZ, double& dScale)
{
	dLon = m_vTargetPos.x();
	dLat = m_vTargetPos.y();
	dHeight = m_vTargetPos.z();

	dRotateX = m_dTargetRotateX;
	dRotateY = m_dTargetRotateY;
	dRotateZ = m_dTargetRotateZ;

	dScale = m_dTargetScale;
}

void DataManager::SetTargetPara(const osg::Vec3d& vecPos, double dRotateX
	, double dRotateY, double dRotateZ, double dScale)
{
	m_vTargetPos = vecPos;

	m_dTargetRotateX = dRotateX;
	m_dTargetRotateY = dRotateY;
	m_dTargetRotateZ = dRotateZ;

	m_dTargetScale = dScale;

	m_pTargetLocalMatrixNode->setMatrix(osg::Matrix());

	osg::Matrix matrixRotate;
	matrixRotate.makeRotate(osg::inDegrees(m_dTargetRotateX), osg::Vec3f(1.0, 0.0, 0.0));

	osg::Matrix matrixRotate2;
	matrixRotate2.makeRotate(osg::inDegrees(m_dTargetRotateY), osg::Vec3f(0.0, 1.0, 0.0));

	osg::Matrix matrixRotate3;
	matrixRotate3.makeRotate(osg::inDegrees(m_dTargetRotateZ), osg::Vec3f(0.0, 0.0, 1.0));

	m_pTargetLocalMatrixNode->postMult(matrixRotate);
	m_pTargetLocalMatrixNode->postMult(matrixRotate2);
	m_pTargetLocalMatrixNode->postMult(matrixRotate3);

	m_pTargetAnimationNode->setPosition(m_vTargetPos);
	m_pTargetAnimationNode->setScale(osg::Vec3d(m_dTargetScale, m_dTargetScale, m_dTargetScale));

	osg::Callback* pCallback = m_pTargetAnimationNode->getUpdateCallback();
	if (pCallback)
	{
		osg::AnimationPathCallback* pAnimationCallback = dynamic_cast<osg::AnimationPathCallback*>(pCallback);
		if (pAnimationCallback)
		{
			osg::AnimationPath* pAnimationPath = pAnimationCallback->getAnimationPath();
			osg::AnimationPath::TimeControlPointMap& pointMap = pAnimationPath->getTimeControlPointMap();
			
			for (osg::AnimationPath::TimeControlPointMap::iterator itr = pointMap.begin(); itr != pointMap.end(); itr ++)
			{
				itr->second.setScale(osg::Vec3d(m_dTargetScale, m_dTargetScale, m_dTargetScale));
			}
		}
	}
}

void DataManager::LoadTargetObject(const QString& strFile)
{
	m_pTargetNode = osgDB::readNodeFile(strFile.toLocal8Bit().data());

	osgUtil::Optimizer optimzer;
	optimzer.optimize(m_pTargetNode);

	int nNumChild = m_pTargetLocalMatrixNode->getNumChildren();
	if (nNumChild > 0)
	{
		osg::Node* pNode = m_pTargetLocalMatrixNode->getChild(0);
		m_pTargetLocalMatrixNode->removeChild(pNode);
	}

	m_pTargetLocalMatrixNode->addChild(m_pTargetNode);

}

osg::Node* createClock()
{
	//表盘的几何节点
	osg::ref_ptr<osg::Geode> clockGeode = new osg::Geode;
	//表盘圈
	osg::ref_ptr<osg::Geometry> clockGeometry = new osg::Geometry;

	//设置线宽
	osg::ref_ptr<osg::LineWidth> lineSize = new osg::LineWidth;
	lineSize->setWidth(2.0);

	osg::ref_ptr<osg::StateSet> stateSet = clockGeode->getOrCreateStateSet();
	//打开线宽属性
	stateSet->setAttributeAndModes(lineSize, osg::StateAttribute::ON);
	clockGeode->addChild(clockGeometry);

	//存放所有圆盘上的点，把这些点连接成直线画成圆盘
	osg::ref_ptr<osg::Vec3Array> allPoints = new osg::Vec3Array;
	//表盘颜色
	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;

	for (double i = 0.0; i < 6.28; i += 0.1)
	{
		colors->push_back(osg::Vec4f(sin(i), cos(i), 0.5, 1.0));
		allPoints->push_back(osg::Vec3(0.001 * sin(i), -0.0, 0.001 * cos(i)));
	}

	//设置顶点
	clockGeometry->setVertexArray(allPoints);

	//画线
	clockGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, allPoints->size()));
	clockGeometry->setColorArray(colors);
	clockGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	return clockGeode.release();
}

void DataManager::ClearPlanePathLine()
{
	osg::Callback* pCallback = m_pRadarBeamLocalMatrixNode->getUpdateCallback();
	OrientationCallback* pOrientationCallback = dynamic_cast<OrientationCallback*>(pCallback);
	
	pOrientationCallback->Clear();
}

void DataManager::ResetAnimationPath()
{
	osg::Callback* pCallback = m_pAerocraftAnimationNode->getUpdateCallback();
	osg::AnimationPathCallback* pAnimationCallback = dynamic_cast<osg::AnimationPathCallback*>(pCallback);
	pAnimationCallback->reset();

	pCallback = m_pTargetAnimationNode->getUpdateCallback();
	pAnimationCallback = dynamic_cast<osg::AnimationPathCallback*>(pCallback);
	pAnimationCallback->reset();
}

void DataManager::LoadRadarBeam(const QString& strFile)
{
	if (m_pRadarBeamLocalMatrixNode == nullptr)
	{
		m_pRadarBeamLocalMatrixNode = new osg::MatrixTransform;
		m_pRoot->addChild(m_pRadarBeamLocalMatrixNode);
	}

	int nNumChild = m_pRadarBeamLocalMatrixNode->getNumChildren();
	if (nNumChild > 0)
	{
		osg::Node* pNode = m_pRadarBeamLocalMatrixNode->getChild(0);
		m_pRadarBeamLocalMatrixNode->removeChild(pNode);
	}

	osg::Switch* pSwitch = new osg::Switch;

	for (int i = 0; i < 16; i ++)
	{
		osg::MatrixTransform* pNewTransform = new osg::MatrixTransform;
		pNewTransform->addChild(createClock()/*m_pRadarBeamNode*/);

		pSwitch->addChild(pNewTransform);
	}

	m_pRadarBeamLocalMatrixNode->addChild(pSwitch);
	m_pRadarBeamLocalMatrixNode->setUpdateCallback(new OrientationCallback);
}

void DataManager::SetRadarBeamLocalMatrix(double dOffsetX, double dOffsetY, double dOffsetZ, double dRotateX
	, double dRotateY, double dRotateZ, double dScale)
{
	m_dRadarOffsetX = dOffsetX;
	m_dRadarOffsetY = dOffsetY;
	m_dRadarOffsetZ = dOffsetZ;

	m_dRadarRotateX = dRotateX;
	m_dRadarRotateY = dRotateY;
	m_dRadarRotateZ = dRotateZ;

	m_dRadarScale = dScale;

	osg::Matrix matrixRotate;
	matrixRotate.makeRotate(osg::inDegrees(m_dRadarRotateX), osg::Vec3f(1.0, 0.0, 0.0));

	osg::Matrix matrixRotate2;
	matrixRotate2.makeRotate(osg::inDegrees(m_dRadarRotateY), osg::Vec3f(0.0, 1.0, 0.0));

	osg::Matrix matrixRotate3;
	matrixRotate3.makeRotate(osg::inDegrees(m_dRadarRotateZ), osg::Vec3f(0.0, 0.0, 1.0));

	osg::Matrix matrixRotate4;
	matrixRotate4.makeTranslate(osg::Vec3d(m_dRadarOffsetX, m_dRadarOffsetY, m_dRadarOffsetZ));

	m_pTargetLocalMatrixNode->postMult(matrixRotate);
	m_pTargetLocalMatrixNode->postMult(matrixRotate2);
	m_pTargetLocalMatrixNode->postMult(matrixRotate3);
	m_pTargetLocalMatrixNode->postMult(matrixRotate4);

	m_pTargetAnimationNode->setPosition(m_vTargetPos);
	m_pTargetAnimationNode->setScale(osg::Vec3d(m_dTargetScale, m_dTargetScale, m_dTargetScale));
}

void DataManager::GetRadarBeamLocalMatrix(double& dOffsetX, double& dOffsetY, double& dOffsetZ, double& dRotateX
	, double& dRotateY, double& dRotateZ, double& dScale)
{
	dOffsetX = m_dRadarOffsetX;
	dOffsetY = m_dRadarOffsetY;
	dOffsetZ = m_dRadarOffsetZ;

	dRotateX = m_dRadarRotateX;
	dRotateY = m_dRadarRotateY;
	dRotateZ = m_dRadarRotateZ;

	dScale = m_dRadarScale;
}