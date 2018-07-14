#pragma once

#include <QtCore/QString>
#include <osg/Node>
#include <osg/MatrixTransform>

class DataManager
{
public:

	enum PathType
	{
		PathType_Circle = 0,
		PathType_Line = 1
	};

	static DataManager* Instance();

	void LoadAerocraft(const QString& strFile);

	void SetAerocraftRotate(float x, float y, float z);

	void GetAerocraftRotate(float& x, float& y, float& z);

	void SetAerocraftScale(double scale);

	double GetAerocraftScale();

	void LoadTargetObject(const QString& strFile);

	//加载雷达波
	void LoadRadarBeam(const QString& strFile);

	void SetRadarBeamLocalMatrix(double dOffsetX, double dOffsetY, double dOffsetZ, double dRotateX
		, double dRotateY, double dRotateZ, double dScale);

	void GetRadarBeamLocalMatrix(double& dOffsetX, double& dOffsetY, double& dOffsetZ, double& dRotateX
		, double& dRotateY, double& dRotateZ, double& dScale);

	osg::Node* GetAerocraftNode();

	osg::Node* GetObservedObjectNode();

	osg::Node* GetTerrainNode();

	osg::Node* GetRadarBeamNode();

	osg::Node* GetRootNode();

	void LoadTerrain();

	void SetCirclePathPara(const osg::Vec3d& vecCenter, double dRadius, double dTime);

	void GetCirclePathPara(osg::Vec3d& vecCenter, double& dRadius, double& dTime);

	void SetLinePathPara(const osg::Vec3d& vecStartPoint, const osg::Vec3d& vecEndPoint, double dTime);

	void GetLinePathPara(osg::Vec3d& startPoint, osg::Vec3d& endPoint, double& dTime);

	void SetPathType(PathType type);

	void GetTargetPara(double& dLon, double& dLat, double& dHeight, double& dRotateX
		, double& dRotateY, double& dRotateZ, double& dScale);

	void SetTargetPara(double dLon, double dLat, double dHeight, double dRotateX
		, double dRotateY, double dRotateZ, double dScale);

protected:

	osg::Group* m_pRoot;

	osg::Node* m_pAerocraftNode;

	osg::Node* m_pTerrainNode;

	osg::MatrixTransform* m_pAerocraftLocalMatrixNode;
	osg::PositionAttitudeTransform* m_pAerocraftAnimationNode;

	//飞行器局部旋转
	float m_dRotateX;
	float m_dRotateY;
	float m_dRotateZ;

	//飞行器缩放倍数
	double m_dScale;

	//环形路径中心
	osg::Vec3d m_circleCenter;
	//环形路径半径
	double m_dCircleRadius;
	//环形路径时间
	double m_dCircleTime;

	//直线路径时间
	double m_dLineTime;
	//直线路径起始点
	osg::Vec3d m_lineStartPoint;
	//直线路径终点
	osg::Vec3d m_lineEndPoint;


	//目标节点
	osg::Node* m_pTargetNode;

	osg::Vec3d m_vTargetPos;

	double m_dTargetRotateX;

	double m_dTargetRotateY;

	double m_dTargetRotateZ;

	//目标局部旋转矩阵
	osg::MatrixTransform* m_pTargetLocalMatrixNode;
	osg::PositionAttitudeTransform* m_pTargetAnimationNode;
	//目标缩放倍数
	double m_dTargetScale;

	//雷达波模型
	osg::Node* m_pRadarBeamNode;

	//雷达波局部矩阵的位移和旋转
	double m_dRadarOffsetX;
	double m_dRadarOffsetY;
	double m_dRadarOffsetZ;
	double m_dRadarRotateX;
	double m_dRadarRotateY;
	double m_dRadarRotateZ;

	double m_dRadarScale;

	osg::MatrixTransform* m_pRadarBeamLocalMatrixNode;

	DataManager();
	virtual ~DataManager();

	DataManager& operator = (const DataManager&);
	DataManager(const DataManager&);
};

