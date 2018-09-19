#include "DataManager.h"
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Geometry>
#include <osg/Geode>
#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>

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

DataManager::DataManager()
{
	m_pRoot = new osg::Group;
	osg::StateSet* pStateSet = m_pRoot->getOrCreateStateSet();
	//pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	m_pAerocraftNode = nullptr;
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
	//pLocalRotate->setMatrix(osg::Matrix::rotate(osg::inDegrees(-90.0f), 0.0f, 0.0f, 1.0f));

	osg::Matrix matrixRotate;
	matrixRotate.makeRotate(osg::inDegrees(m_dRotateX), osg::Vec3f(1.0, 0.0, 0.0));

	osg::Matrix matrixRotate2;
	matrixRotate2.makeRotate(osg::inDegrees(m_dRotateY), osg::Vec3f(0.0, 1.0, 0.0));

	osg::Matrix matrixRotate3;
	matrixRotate3.makeRotate(osg::inDegrees(m_dRotateZ), osg::Vec3f(0.0, 0.0, 1.0));

	m_pAerocraftLocalMatrixNode->postMult(matrixRotate);
	m_pAerocraftLocalMatrixNode->postMult(matrixRotate2);
	m_pAerocraftLocalMatrixNode->postMult(matrixRotate3);

	//创建位置变换节点pat1,缩放
	m_pAerocraftAnimationNode = new osg::PositionAttitudeTransform;

	m_pAerocraftAnimationNode->setPosition(osg::Vec3d(121, 22.0, 0.1));
	m_pAerocraftAnimationNode->setScale(osg::Vec3(m_dScale, m_dScale, m_dScale));

	m_pAerocraftAnimationNode->addChild(m_pAerocraftLocalMatrixNode);

	osg::Vec3d scale(m_dScale, m_dScale, m_dScale);

	osg::AnimationPath* animationPath = createCircleAnimationPath(m_circleCenter, scale, m_dCircleRadius, m_dCircleTime);
	m_pAerocraftAnimationNode->setUpdateCallback(new osg::AnimationPathCallback(animationPath, 0.0, 1.0));

	m_pRoot->addChild(m_pAerocraftAnimationNode);

	m_dTargetRotateX = 0.0;
	m_dTargetRotateY = 0.0;
	m_dTargetRotateZ = 0.0;
	m_dTargetScale = 1.0;

	m_pTargetLocalMatrixNode = new osg::MatrixTransform;
	m_pTargetAnimationNode = new osg::PositionAttitudeTransform;
	m_pTargetAnimationNode->addChild(m_pTargetLocalMatrixNode);
	m_pRoot->addChild(m_pTargetAnimationNode);

	m_pRadarBeamNode = nullptr;

	//雷达波局部矩阵的位移和旋转
	m_dRadarOffsetX = 0.0;
	m_dRadarOffsetY = 0.0;
	m_dRadarOffsetZ = 0.0;
	m_dRadarRotateX = 0.0;
	m_dRadarRotateY = 0.0;
	m_dRadarRotateZ = 0.0;

	m_dRadarScale = 1.0;

	m_pRadarBeamLocalMatrixNode = new osg::MatrixTransform;
	m_pTargetAnimationNode->addChild(m_pRadarBeamLocalMatrixNode);

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

		pTerrainGroup->addChild(osgDB::readNodeFile("D:/L19/world.ive"));
		return;

		pTerrainGroup->addChild(osgDB::readNodeFile("D:/rcsmodel/dunhuang.ive"));
		pTerrainGroup->addChild(osgDB::readNodeFile("D:/rcsmodel/dunhuang(mubiaoqu).ive"));
		pTerrainGroup->addChild(osgDB::readNodeFile("D:/rcsmodel/dunhuang3.ive"));
		pTerrainGroup->addChild(osgDB::readNodeFile("D:/rcsmodel/hami(jichangqu).ive"));
		pTerrainGroup->addChild(osgDB::readNodeFile("D:/rcsmodel/hami(mubiaoqu).ive"));
		pTerrainGroup->addChild(osgDB::readNodeFile("D:/rcsmodel/jinzhou.ive"));
		pTerrainGroup->addChild(osgDB::readNodeFile("D:/rcsmodel/kuerle.ive"));
		pTerrainGroup->addChild(osgDB::readNodeFile("D:/rcsmodel/liaoningchaoyang.ive"));
		pTerrainGroup->addChild(osgDB::readNodeFile("D:/rcsmodel/qiemo.ive"));
		pTerrainGroup->addChild(osgDB::readNodeFile("D:/rcsmodel/xilinhaote.ive"));
		pTerrainGroup->addChild(osgDB::readNodeFile("D:/osg3.2.0/taiwan/iso.ive"));
	}
}

void DataManager::LoadAerocraft(const QString& strFile)
{
	//m_pAerocraftNode = osgDB::readNodeFile("glider.osg"/*"su27.IVE"*/); 
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

	osg::AnimationPath* animationPath = createCircleAnimationPath(m_circleCenter, scale, m_dCircleRadius, m_dCircleTime);
	m_pAerocraftAnimationNode->setUpdateCallback(new osg::AnimationPathCallback(animationPath, 0.0, 1.0));
}

double DataManager::GetAerocraftScale()
{
	return m_dScale;
}

osg::Node* DataManager::GetAerocraftNode()
{
	return m_pAerocraftNode;
}

osg::Node* DataManager::GetObservedObjectNode()
{
	return nullptr;
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

void DataManager::SetTargetPara(double dLon, double dLat, double dHeight, double dRotateX
	, double dRotateY, double dRotateZ, double dScale)
{
	m_vTargetPos.x() = dLon;
	m_vTargetPos.y() = dLat;
	m_vTargetPos.z() = dHeight;

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

void DataManager::LoadRadarBeam(const QString& strFile)
{
	m_pRadarBeamNode = osgDB::readNodeFile(strFile.toUtf8().data());

	osgUtil::Optimizer optimzer;
	optimzer.optimize(m_pRadarBeamNode);

	int nNumChild = m_pRadarBeamLocalMatrixNode->getNumChildren();
	if (nNumChild > 0)
	{
		osg::Node* pNode = m_pRadarBeamLocalMatrixNode->getChild(0);
		m_pRadarBeamLocalMatrixNode->removeChild(pNode);
	}

	m_pRadarBeamLocalMatrixNode->addChild(m_pRadarBeamNode);
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

	m_pRadarBeamLocalMatrixNode->setMatrix(osg::Matrix());

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