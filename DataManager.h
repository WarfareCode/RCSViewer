#pragma once

#include <QtCore/QString>
#include <osg/Node>
#include <osg/MatrixTransform>

class DataManager
{
public:

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

protected:

	osg::Group* m_pRoot;

	osg::Node* m_pAerocraftNode;

	osg::Node* m_pTerrainNode;

	osg::MatrixTransform* m_pAerocraftLocalMatrixNode;

	float m_dRotateX;
	float m_dRotateY;
	float m_dRotateZ;
	double m_dScale;

	osg::PositionAttitudeTransform* m_pAerocraftAnimationNode;

	DataManager();
	virtual ~DataManager();

	DataManager& operator = (const DataManager&);
	DataManager(const DataManager&);
};

