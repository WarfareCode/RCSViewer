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

	void LoadObservedObject(const QString& strFile);

	osg::Node* GetAerocraftNode();

	osg::Node* GetObservedObjectNode();

	osg::Node* GetTerrainNode();

	osg::Node* GetRootNode();

	void LoadTerrain();

	void SetCirclePathPara(const osg::Vec3d& vecCenter, double dRadius, double dTime);

	void GetCirclePathPara(osg::Vec3d& vecCenter, double& dRadius, double& dTime);

	void SetLinePathPara(const osg::Vec3d& vecStartPoint, const osg::Vec3d& vecEndPoint, double dTime);

	void GetLinePathPara(osg::Vec3d& startPoint, osg::Vec3d& endPoint, double& dTime);

	void SetPathType(PathType type);

protected:

	osg::Group* m_pRoot;

	osg::Node* m_pAerocraftNode;

	osg::Node* m_pTerrainNode;

	osg::MatrixTransform* m_pAerocraftLocalMatrixNode;

	float m_dRotateX;
	float m_dRotateY;
	float m_dRotateZ;
	double m_dScale;
	osg::Vec3d m_circleCenter;
	double m_dCircleRadius;
	double m_dCircleTime;
	double m_dLineTime;
	osg::Vec3d m_lineStartPoint;
	osg::Vec3d m_lineEndPoint;

	osg::PositionAttitudeTransform* m_pAerocraftAnimationNode;

	DataManager();
	virtual ~DataManager();

	DataManager& operator = (const DataManager&);
	DataManager(const DataManager&);
};

