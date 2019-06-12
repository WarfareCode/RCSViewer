#pragma once

#include <QtCore/QString>
#include <osg/Node>
#include <osg/MatrixTransform>

static char * vertexShader = {
	"void main(void ){\n"
	"gl_TexCoord[0] = gl_MultiTexCoord0;\n"
	"gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;\n"
	"}\n"
};
static char * fragShader = {
	"uniform sampler2D sampler0;\n"
	"void main(void){\n"
	"gl_FragColor = texture2D(sampler0, gl_TexCoord[0].st);\n"
	"if(gl_FragColor.r == 0.0)\n"
	"gl_FragColor.a = 0.0;\n"

	"}\n"
};

class DataManager
{
public:

	enum PathType
	{
		PathType_Circle = 0,
		PathType_Line = 1
	};

	static DataManager* Instance();

	bool LoadDataAndDisplay(QString gpsfile, QString targpsfile, QString rcsfile, QString video);

	void LoadAerocraft(const QString& strFile);

	void SetAerocraftRotate(float x, float y, float z);

	void GetAerocraftRotate(float& x, float& y, float& z);

	void SetAerocraftScale(double scale);

	double GetAerocraftScale();

	void LoadTargetObject(const QString& strFile);

	//�����״ﲨ
	void LoadRadarBeam(const QString& strFile);

	void SetRadarBeamLocalMatrix(double dOffsetX, double dOffsetY, double dOffsetZ, double dRotateX
		, double dRotateY, double dRotateZ, double dScale);

	void GetRadarBeamLocalMatrix(double& dOffsetX, double& dOffsetY, double& dOffsetZ, double& dRotateX
		, double& dRotateY, double& dRotateZ, double& dScale);

	osg::Node* GetAerocraftNode();

	osg::PositionAttitudeTransform* GetPlaneParentNode();

	osg::Node* GetTargetObjectNode();

	osg::Node* GetTerrainNode();

	osg::Node* GetRadarBeamNode();

	osg::Node* GetRootNode();

	osg::Geode* GetPlanePath(){ return m_pAeroPathLine; }

	void LoadTerrain();

	void SetCirclePathPara(const osg::Vec3d& vecCenter, double dRadius, double dTime);

	void GetCirclePathPara(osg::Vec3d& vecCenter, double& dRadius, double& dTime);

	void SetLinePathPara(const osg::Vec3d& vecStartPoint, const osg::Vec3d& vecEndPoint, double dTime);

	void GetLinePathPara(osg::Vec3d& startPoint, osg::Vec3d& endPoint, double& dTime);

	void SetPathType(PathType type);

	void GetTargetPara(double& dLon, double& dLat, double& dHeight, double& dRotateX
		, double& dRotateY, double& dRotateZ, double& dScale);

	void SetTargetPara(const osg::Vec3d& vecPos, double dRotateX
		, double dRotateY, double dRotateZ, double dScale);

	void GetPlanePathEnv(double& dx1, double& dy1, double& dx2, double& dy2, double& dH);

	//��շɻ��켣��
	void ClearPlanePathLine();

	//��ֵ����λ��
	void ResetAnimationPath();

	//����¼���ļ��ı���·����
	void SetScreenCaptureFilePath(QString& strPath){ m_strScreenCaptureFilePath = strPath; }
	QString GetScreenCaptureFilePath(){ return m_strScreenCaptureFilePath; }

protected:

	QString m_strScreenCaptureFilePath;

	osg::Group* m_pRoot;

	//�ɻ����й켣��
	osg::Geode* m_pAeroPathLine;

	osg::Node* m_pAerocraftNode;

	osg::Node* m_pTerrainNode;

	osg::MatrixTransform* m_pAerocraftLocalMatrixNode;
	osg::PositionAttitudeTransform* m_pAerocraftAnimationNode;

	//�������ֲ���ת
	float m_dRotateX;
	float m_dRotateY;
	float m_dRotateZ;

	//���������ű���
	double m_dScale;

	//�������켣����
	PathType m_ePlanePathType;

	//����·������
	osg::Vec3d m_circleCenter;
	//����·���뾶
	double m_dCircleRadius;
	//����·��ʱ��
	double m_dCircleTime;

	//ֱ��·��ʱ��
	double m_dLineTime;
	//ֱ��·����ʼ��
	osg::Vec3d m_lineStartPoint;
	//ֱ��·���յ�
	osg::Vec3d m_lineEndPoint;


	//Ŀ��ڵ�
	osg::Node* m_pTargetNode;

	osg::Vec3d m_vTargetPos;

	double m_dTargetRotateX;

	double m_dTargetRotateY;

	double m_dTargetRotateZ;

	//Ŀ��ֲ���ת����
	osg::MatrixTransform* m_pTargetLocalMatrixNode;
	osg::PositionAttitudeTransform* m_pTargetAnimationNode;
	//Ŀ�����ű���
	double m_dTargetScale;

	//�״ﲨģ��
	osg::Node* m_pRadarBeamNode;

	//�״ﲨ�ֲ������λ�ƺ���ת
	double m_dRadarOffsetX;
	double m_dRadarOffsetY;
	double m_dRadarOffsetZ;
	double m_dRadarRotateX;
	double m_dRadarRotateY;
	double m_dRadarRotateZ;

	double m_dRadarScale;

	osg::MatrixTransform* m_pRadarBeamLocalMatrixNode;

	//�ɻ����й켣�ķ�Χ
	double m_dLeft;
	double m_dTop;
	double m_dRight;
	double m_dBottom;
	double m_dH;

	DataManager();
	virtual ~DataManager();

	DataManager& operator = (const DataManager&);
	DataManager(const DataManager&);
};

