#include "DataManager.h"
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
#include <QtGui/QVector3D>
#include <QtCore/QFile>
#include <QtCore/QVector>
#include <QtCore/QTime>

static char * vertexShader = {
	"void main(void ){\n"
	"gl_TexCoord[0] = gl_MultiTexCoord0;\n"
	"gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;\n"
	"}\n"
};
static char * fragShader = {
	"uniform sampler2D sampler0;\n"
	"uniform vec4 mcolor;\n"
	"void main(void){\n"
	"gl_FragColor = texture2D(sampler0, gl_TexCoord[0].st);\n"
	"if(gl_FragColor.r < 0.1)\n"
	"gl_FragColor.a = 0.0;\n"

	"}\n"
};

DataManager* g_DataManager = nullptr;

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
		//osg::Quat rotation(osg::Quat(roll, osg::Vec3d(0.0, 1.0, 0.0))*osg::Quat(-(yaw + osg::inDegrees(90.0f)), osg::Vec3d(0.0, 0.0, 1.0)));
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

			// 		if (bIni == false)
			// 		{
			// 			timeStart = QTime::fromString(strList[1], "hh:mm:ss");
			// 			bIni = true;
			// 		}
			// 
			// 		QTime time = QTime::fromString(strList[1], "hh:mm:ss");
			// 		double dTime = time.secsTo(timeStart);
			// 
			// 		vecTime.push_back(dTime);
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
	for (int i = 0; i < nSize; i ++)
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
	m_dCircleTime = 200.0;

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

	//osg::AnimationPath* animationPath = createCircleAnimationPath(m_circleCenter, scale, m_dCircleRadius, m_dCircleTime);
	osg::AnimationPath* animationPath = createPlaneAnimationPath("c:/airport_gps.dat", scale, 400/*m_dCircleTime*/);
	m_pAerocraftAnimationNode->setUpdateCallback(new osg::AnimationPathCallback(animationPath, 0.0, 1.0));

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
	osg::AnimationPath* targetAnimationPath = createTargetAnimationPath("c:/a.txt", scaleTarget, 400.0);
	m_pTargetAnimationNode->setUpdateCallback(new osg::AnimationPathCallback(targetAnimationPath));

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

DataManager* DataManager::Instance()
{
	if (g_DataManager == nullptr)
	{
		g_DataManager = new DataManager;
	}

	return g_DataManager;
}

void DataManager::LoadTerrain()
{
	if (m_pTerrainNode == nullptr)
	{
		osg::Group* pTerrainGroup = new osg::Group;
		m_pTerrainNode = pTerrainGroup;
		m_pRoot->addChild(m_pTerrainNode);

		//m_pTerrainNode = osgDB::readNodeFile("D:/rcsmodel/qiemo.ive"/*"D:/L19/ttt.ive"*//*"D:/osg3.2.0/taiwan/iso.ive"*/);
		//m_pRoot->addChild(osgDB::readNodeFile("D:/osg3.2.0/taiwan/iso.ive"));
		//m_pRoot->addChild(osgDB::readNodeFile("D:/rcsmodel/ooo.ive"));

		osg::Node* pNode = osgDB::readNodeFile("D:/c/taiwan.ive");
		pTerrainGroup->addChild(pNode);

		osg::StateSet * ss = pNode/*pTerrainGroup*/->getOrCreateStateSet();

		if (1)
		{
// 			osg::ref_ptr<osg::BlendFunc>blendFunc = new osg::BlendFunc();
// 			blendFunc->setSource(osg::BlendFunc::SRC_ALPHA);
// 			blendFunc->setDestination(osg::BlendFunc::ONE_MINUS_CONSTANT_ALPHA);
// 			ss->setAttributeAndModes(blendFunc);
// 			ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);//取消深度测试
			if (1)
			{
				osg::Program * program = new osg::Program;
				program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShader));
				program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShader));

				osg::ref_ptr<osg::Uniform> sampler0 = new osg::Uniform("sampler0", 0);
				ss->addUniform(sampler0.get());

				osg::ref_ptr<osg::Uniform> mcolor = new osg::Uniform("mcolor", osg::Vec4(1.0, 0.0, 0.0, 0.1));
				//mcolor->setUpdateCallback(new MColorCallback(dTime));
				ss->addUniform(mcolor.get());

				ss->setAttributeAndModes(program, osg::StateAttribute::OVERRIDE);
				ss->setMode(GL_BLEND, osg::StateAttribute::ON);
				ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
				ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
			}
		}

		//pTerrainGroup->addChild(osgDB::readNodeFile("D:/rcsmodel/west.ive"));
		//pTerrainGroup->addChild(osgDB::readNodeFile("D:/rcsmodel/east_little.ive"));

		//return;

		pNode = osgDB::readNodeFile("D:/rcsmodel/sanya_clip.ive");
		pNode->setStateSet(ss);
		pTerrainGroup->addChild(pNode);

		pNode = osgDB::readNodeFile("D:/rcsmodel/huludao_clip.ive");
		pNode->setStateSet(ss);
		pTerrainGroup->addChild(pNode);

		pNode = osgDB::readNodeFile("D:/rcsmodel/zhoushan_clip.ive");
		pNode->setStateSet(ss);
		pTerrainGroup->addChild(pNode);

		pNode = osgDB::readNodeFile("D:/rcsmodel/qinhuangdao_clip.ive");
		pNode->setStateSet(ss);
		pTerrainGroup->addChild(pNode);

		pNode = osgDB::readNodeFile("D:/rcsmodel/dalianchanghai_clip.ive");
		pNode->setStateSet(ss);
		pTerrainGroup->addChild(pNode);

		pNode = osgDB::readNodeFile("D:/rcsmodel/dunhuang.ive");
		osg::StateSet* stateset = pNode->getOrCreateStateSet();
		stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
		stateset->setRenderBinDetails(11, "RenderBin");
		pTerrainGroup->addChild(pNode);


		pNode = osgDB::readNodeFile("D:/rcsmodel/dunhuang(mubiaoqu).ive");
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		pNode = osgDB::readNodeFile("D:/rcsmodel/dunhuang3.ive");
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		pNode = osgDB::readNodeFile("D:/rcsmodel/hami(jichangqu).ive");
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		pNode = osgDB::readNodeFile("D:/rcsmodel/hami(mubiaoqu).ive");
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		pNode = osgDB::readNodeFile("D:/rcsmodel/jinzhou.ive");
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		pNode = osgDB::readNodeFile("D:/rcsmodel/kuerle.ive");
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		pNode = osgDB::readNodeFile("D:/rcsmodel/liaoningchaoyang.ive");
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		pNode = osgDB::readNodeFile("D:/rcsmodel/qiemo.ive");
		pNode->setStateSet(stateset);
		pTerrainGroup->addChild(pNode);

		pNode = osgDB::readNodeFile("D:/rcsmodel/xilinhaote.ive");
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

	osg::AnimationPath* animationPath = nullptr;
	if (m_ePlanePathType == PathType_Circle)
	{
		animationPath = createCircleAnimationPath(m_circleCenter, scale, m_dCircleRadius, m_dCircleTime);
	}
	else if (m_ePlanePathType == PathType_Line)
	{
		animationPath = createLineAnimationPath(m_lineStartPoint, m_lineEndPoint, scale, m_dLineTime);
	}

	m_pAerocraftAnimationNode->setUpdateCallback(new osg::AnimationPathCallback(animationPath, 0.0, 1.0));
}

double DataManager::GetAerocraftScale()
{
	return m_dScale;
}

osg::Node* DataManager::GetRadarBeamNode()
{
	return m_pRadarBeamNode;
}

osg::Node* DataManager::GetAerocraftNode()
{
	return m_pAerocraftNode;
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
// 	m_vTargetPos.x() = dLon;
// 	m_vTargetPos.y() = dLat;
// 	m_vTargetPos.z() = dHeight;

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

	// 		osgUtil::Optimizer optimzer;
	// 		optimzer.optimize(rootnode);
}

class OrientationCallback : public osg::NodeCallback
{
public:

	OrientationCallback()
	{
		m_dLastTime = 0.0;
		m_pArray = new osg::Vec3Array();
	}

	~OrientationCallback(){}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		if (nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
		{
			osg::MatrixTransform* pTransform = dynamic_cast<osg::MatrixTransform*>(node);

			osg::Node* pTargetNode = DataManager::Instance()->GetTargetObjectNode();
			osg::Node* pPlaneNode = DataManager::Instance()->GetAerocraftNode();
			osg::Node* pBeamNode = DataManager::Instance()->GetRadarBeamNode();
			
			osg::Vec3 planePos = pPlaneNode->getBound().center() * osg::computeLocalToWorld(pPlaneNode->getParentalNodePaths()[0]);
			osg::Vec3 targetPos = pTargetNode->getBound().center() * osg::computeLocalToWorld(pTargetNode->getParentalNodePaths()[0]);
			
			osg::Matrix matrix;
			matrix.makeTranslate(planePos);
			pTransform->setMatrix(matrix);

			osg::Vec3 vec0(0, 0, -1);
			osg::Vec3 vec1 = targetPos - planePos;

			osg::Geode* pGeode = dynamic_cast<osg::Geode*>(pBeamNode);

			osg::BoundingSphere bound = pBeamNode->getBound();

			osg::Matrix matr;
			matr.makeTranslate(/*-bound.center()*/osg::Vec3(0.0, 0.0, -(3.0*bound.center().z())));
			//matr.makeTranslate(-bound.center() * 2.0);

			osg::Matrix maro;
			maro.makeRotate(vec0, vec1);

			matr.postMult(maro);

			osg::MatrixTransform* pSubTransform = dynamic_cast<osg::MatrixTransform*>(pTransform->getChild(0));
			pSubTransform->setMatrix(matr);

			//绘制航线
			double dTime = nv->getFrameStamp()->getSimulationTime();

			if ((dTime - m_dLastTime) > 0.1)
			{
				m_dLastTime = dTime;
				m_pArray->push_back(planePos);

				osg::Geode* pPlanePath = DataManager::Instance()->GetPlanePath();
	
				osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
				{
					geometry->setVertexArray(m_pArray.get());

					osg::Vec4Array* colors = new osg::Vec4Array;
					colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
					geometry->setColorArray(colors, osg::Array::BIND_OVERALL);

					geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, m_pArray->size()));
				}

				osg::Drawable* pDrawable = pPlanePath->getDrawable(0);
				pPlanePath->removeDrawable(pDrawable);
				pPlanePath->addDrawable(geometry.get());
				//m_pAeroPathLine->addDrawable(geometry.get());
			}
		}

		traverse(node, nv);
	}

protected:

	double m_dLastTime;

	osg::ref_ptr<osg::Vec3Array> m_pArray;

};

void DataManager::LoadRadarBeam(const QString& strFile)
{
	//m_pRadarBeamNode = osgDB::readNodeFile(strFile.toUtf8().data());

	{
		osg::ref_ptr<osg::Cone>  cone = new osg::Cone;
		osg::ref_ptr<osg::ShapeDrawable> shap = new osg::ShapeDrawable(cone);
		osg::ref_ptr<osg::Geode> geode = new osg::Geode;
		geode->addDrawable(shap);

		m_pRadarBeamNode = geode;
		cone->setHeight(2);
		cone->setRadius(0.15);
		shap->setColor(osg::Vec4(0.0, 1.0, 0.0, 0.3));

		osg::ref_ptr<osg::StateSet> stateset = geode->getOrCreateStateSet();
		stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
		stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		osg::ref_ptr<osg::PolygonMode> polyMode = new osg::PolygonMode(osg::PolygonMode::BACK, osg::PolygonMode::FILL);
		stateset->setAttribute(polyMode);
		geode.release();
	}

	osgUtil::Optimizer optimzer;
	optimzer.optimize(m_pRadarBeamNode);

	if (m_pRadarBeamLocalMatrixNode == nullptr)
	{
		m_pRadarBeamLocalMatrixNode = new osg::MatrixTransform;
		//m_pAerocraftLocalMatrixNode->addChild(m_pRadarBeamLocalMatrixNode);
		m_pRoot->addChild(m_pRadarBeamLocalMatrixNode);
	}

	int nNumChild = m_pRadarBeamLocalMatrixNode->getNumChildren();
	if (nNumChild > 0)
	{
		osg::Node* pNode = m_pRadarBeamLocalMatrixNode->getChild(0);
		m_pRadarBeamLocalMatrixNode->removeChild(pNode);
	}

	osg::BoundingSphere bound = m_pAerocraftNode->getBound();
	osg::BoundingSphere bound2 = m_pRadarBeamNode->getBound();

	osg::MatrixTransform* pNewTransform = new osg::MatrixTransform;
	pNewTransform->addChild(m_pRadarBeamNode);

	m_pRadarBeamLocalMatrixNode->addChild(pNewTransform);
//	

	osg::Matrix matrix = osg::Matrix::translate(bound.center() - bound2.center());
 	osg::Matrix matrixRotate;
 	//matrixRotate.makeRotate(osg::inDegrees(45.0), osg::Vec3f(1.0, 0.0, 0.0));

	matrixRotate.postMult(matrix);
	m_pRadarBeamLocalMatrixNode->setMatrix(matrixRotate);

	m_pRadarBeamLocalMatrixNode->setUpdateCallback(new OrientationCallback);

//	m_pRadarBeamLocalMatrixNode->postMult(matrixRotate3);

	//经度121.038, 纬度23.613， 高度0.1， 半径0.8
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