#ifndef AUTOSWITCHMATRIXMANIPULATOR_H
#define AUTOSWITCHMATRIXMANIPULATOR_H

#include "osgGA/CameraManipulator"

namespace osgGA
{
	class AutoSwitchMatrixManipulator : public CameraManipulator
	{
	public:
		AutoSwitchMatrixManipulator();
		~AutoSwitchMatrixManipulator();

		virtual const char* className() const { return "AutoSwitchMatrixManipulator"; }

		//virtual void home(const GUIEventAdapter& ee, GUIActionAdapter& aa){};

		virtual bool handle(const GUIEventAdapter& ea, GUIActionAdapter& us);

		void AddManipulator(CameraManipulator *cm);

		virtual void setCoordinateFrameCallback(CoordinateFrameCallback* cb);

		/** Get the FusionDistanceMode. Used by SceneView for setting up stereo convergence.*/
		virtual osgUtil::SceneView::FusionDistanceMode getFusionDistanceMode() const { return m_vecManipulator[m_nCurrentIndex]->getFusionDistanceMode(); }

		/** Get the FusionDistanceValue. Used by SceneView for setting up stereo convergence.*/
		virtual float getFusionDistanceValue() const { return m_vecManipulator[m_nCurrentIndex]->getFusionDistanceValue(); }

		virtual void setNode(osg::Node* n);

		virtual const osg::Node* getNode() const        { return m_vecManipulator[m_nCurrentIndex]->getNode(); }

		virtual osg::Node* getNode()                    { return m_vecManipulator[m_nCurrentIndex]->getNode(); }

		virtual void setHomePosition(const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up, bool autoComputeHomePosition = false);

		virtual void setAutoComputeHomePosition(bool flag);

		virtual void computeHomePosition();

		virtual void finishAnimation();

		virtual void home(const GUIEventAdapter& ee, GUIActionAdapter& aa);

		virtual void init(const GUIEventAdapter& ee, GUIActionAdapter& aa) { if (m_vecManipulator[m_nCurrentIndex].valid()) m_vecManipulator[m_nCurrentIndex]->init(ee, aa); }

		/** Set the position of the matrix manipulator using a 4x4 Matrix.*/
		virtual void setByMatrix(const osg::Matrixd& matrix) { m_vecManipulator[m_nCurrentIndex]->setByMatrix(matrix); }

		/** set the position of the matrix manipulator using a 4x4 Matrix.*/
		virtual void setByInverseMatrix(const osg::Matrixd& matrix) { m_vecManipulator[m_nCurrentIndex]->setByInverseMatrix(matrix); }

		/** get the position of the manipulator as 4x4 Matrix.*/
		virtual osg::Matrixd getMatrix() const { return m_vecManipulator[m_nCurrentIndex]->getMatrix(); }

		/** get the position of the manipulator as a inverse matrix of the manipulator, typically used as a model view matrix.*/
		virtual osg::Matrixd getInverseMatrix() const { return m_vecManipulator[m_nCurrentIndex]->getInverseMatrix(); }

	private:

		double m_dLastTimeStamp;

		std::vector<osg::ref_ptr<CameraManipulator>> m_vecManipulator;
		int m_nCurrentIndex;
	};
}



#endif // AUTOSWITCHMATRIXMANIPULATOR_H
