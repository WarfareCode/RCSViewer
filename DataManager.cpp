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
	m_pAerocraftNode = nullptr;
	m_pTerrainNode = nullptr;

	m_dRotateX = -90.0f;
	m_dRotateY = 180.0f;
	m_dRotateZ = 270.0f;

	m_dScale = 1.0;

	m_circleCenter = osg::Vec3d(121.038, 23.613, 0.1);
	m_dScale = 0.05;
	m_dCircleRadius = 1.0;
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
		m_pTerrainNode = osgDB::readNodeFile("D:/osg3.2.0/taiwan/iso.ive");
		m_pRoot->addChild(m_pTerrainNode);
	}
}

void DataManager::LoadAerocraft(const QString& strFile)
{
	//m_pAerocraftNode = osgDB::readNodeFile("glider.osg"/*"su27.IVE"*/); 
	m_pAerocraftNode = osgDB::readNodeFile(strFile.toUtf8().data()/*"su27.IVE"*/);

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

void DataManager::LoadObservedObject(const QString& strFile)
{
	// 		osgUtil::Optimizer optimzer;
	// 		optimzer.optimize(rootnode);
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