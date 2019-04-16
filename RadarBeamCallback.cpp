#include "RadarBeamCallback.h"
#include <osg/NodeVisitor>

RadarBeamCallback::RadarBeamCallback()
{
	m_dLastTime = 0.0;
}

RadarBeamCallback::~RadarBeamCallback()
{

}

void RadarBeamCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	if (nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
	{
		double dTime = nv->getFrameStamp()->getSimulationTime();
		if ((dTime - m_dLastTime) > 0.1)
		{
			m_dLastTime = dTime;
		}
	}
}