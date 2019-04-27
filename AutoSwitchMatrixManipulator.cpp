#include "AutoSwitchMatrixManipulator.h"
#include <osgGA/AnimationPathManipulator>

namespace osgGA
{
	AutoSwitchMatrixManipulator::AutoSwitchMatrixManipulator()
	{
		m_dLastTimeStamp = -1.0;
		m_nCurrentIndex = 0;
	}

	AutoSwitchMatrixManipulator::~AutoSwitchMatrixManipulator()
	{

	}

	void AutoSwitchMatrixManipulator::AddManipulator(CameraManipulator *cm)
	{
		m_vecManipulator.push_back(osg::ref_ptr<CameraManipulator>(cm));
	}

	void AutoSwitchMatrixManipulator::setNode(osg::Node* node)
	{
		for (auto& itr : m_vecManipulator)
		{
			itr->setNode(node);
		}
	}

	void AutoSwitchMatrixManipulator::setHomePosition(const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up, bool autoComputeHomePosition)
	{
		CameraManipulator::setHomePosition(eye, center, up, autoComputeHomePosition);
		for (auto& itr : m_vecManipulator)
		{
			itr->setHomePosition(eye, center, up, autoComputeHomePosition);
		}
	}

	void AutoSwitchMatrixManipulator::setAutoComputeHomePosition(bool flag)
	{
		_autoComputeHomePosition = flag;
		for (auto& itr : m_vecManipulator)
		{
			itr->setAutoComputeHomePosition(flag);
		}
	}

	void AutoSwitchMatrixManipulator::computeHomePosition()
	{
		for (auto& itr : m_vecManipulator)
		{
			itr->computeHomePosition();
		}
	}

	void AutoSwitchMatrixManipulator::finishAnimation()
	{
		for (auto& itr : m_vecManipulator)
		{
			itr->finishAnimation();
		}
	}

	void AutoSwitchMatrixManipulator::home(const GUIEventAdapter& ee, GUIActionAdapter& aa)
	{
		// call home for all child manipulators
		// (this can not be done just for current manipulator,
		// because it is not possible to transfer some manipulator
		// settings across manipulators using just MatrixManipulator interface
		// (one problematic variable is for example OrbitManipulator::distance
		// that can not be passed by setByMatrix method),
		// thus we have to call home on all of them)
		for (auto& itr : m_vecManipulator)
		{
			itr->home(ee, aa);
		}
	}

	void AutoSwitchMatrixManipulator::setCoordinateFrameCallback(CoordinateFrameCallback* cb)
	{
		_coordinateFrameCallback = cb;

		for (auto& itr : m_vecManipulator)
		{
			itr->setCoordinateFrameCallback(cb);
		}
	}

	bool AutoSwitchMatrixManipulator::handle(const GUIEventAdapter& ea, GUIActionAdapter& aa)
	{
		if (ea.getHandled())
			return false;

		if (m_dLastTimeStamp == -1.0)
		{
			m_dLastTimeStamp = ea.getTime();
		}

		if (ea.getEventType() == GUIEventAdapter::FRAME)
		{
			double dInterval = 3.0;
			osgGA::AnimationPathManipulator* pAnimationManipulator = 
				dynamic_cast<osgGA::AnimationPathManipulator*>(m_vecManipulator[m_nCurrentIndex].get());

			if (pAnimationManipulator)
			{
				dInterval = pAnimationManipulator->getAnimationPath()->getPeriod();
			}

			if ((ea.getTime() - m_dLastTimeStamp) > dInterval)
			{
				m_dLastTimeStamp = ea.getTime();

				m_nCurrentIndex++;
				if (m_nCurrentIndex >= m_vecManipulator.size())
				{
					m_nCurrentIndex = 0;
				}

				osgGA::AnimationPathManipulator* pAnimationManipulator =
					dynamic_cast<osgGA::AnimationPathManipulator*>(m_vecManipulator[m_nCurrentIndex].get());
				if (pAnimationManipulator)
				{
					pAnimationManipulator->setTimeOffset(ea.getTime() * -1.0);
				}
			}
		}

		switch (ea.getEventType())
		{
		case GUIEventAdapter::MOVE:
		case GUIEventAdapter::DRAG:
		case GUIEventAdapter::PUSH:
		case GUIEventAdapter::RELEASE:
		case GUIEventAdapter::KEYDOWN:
		case GUIEventAdapter::KEYUP:
		case GUIEventAdapter::SCROLL:
			return true;
		}

		return m_vecManipulator[m_nCurrentIndex]->handle(ea, aa);
	}
}

