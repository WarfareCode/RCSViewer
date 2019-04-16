#include "PlaneLOD.h"
#include <algorithm>

void get_panel(osg::Vec3d p1, osg::Vec3d p2, osg::Vec3d p3, double &a, double &b, double &c, double &d)
{
	a = (p2.y() - p1.y())*(p3.z() - p1.z()) - (p2.z() - p1.z())*(p3.y() - p1.y());
	b = (p2.z() - p1.z())*(p3.x() - p1.x()) - (p2.x() - p1.x())*(p3.z() - p1.z());
	c = (p2.x() - p1.x())*(p3.y() - p1.y()) - (p2.y() - p1.y())*(p3.x() - p1.x());
	d = 0 - (a * p1.x() + b*p1.y() + c*p1.z());

}

PlaneLOD::PlaneLOD()
{

}

PlaneLOD::~PlaneLOD()
{

}

void PlaneLOD::traverse(osg::NodeVisitor& nv)
{
	switch (nv.getTraversalMode())
	{
	case(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) :
		std::for_each(_children.begin(), _children.end(), osg::NodeAcceptOp(nv));
		break;
	case(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN) :
	{
		float required_range = 0;
		if (_rangeMode == DISTANCE_FROM_EYE_POINT)
		{
			required_range = nv.getDistanceToViewPoint(getCenter(), true);
		}

		unsigned int numChildren = _children.size();
		if (_rangeList.size() < numChildren) numChildren = _rangeList.size();

		for (unsigned int i = 0; i < numChildren; ++i)
		{
			if (_rangeList[i].first <= required_range && required_range < _rangeList[i].second)
			{
				_children[i]->accept(nv);
			}
		}
		break;
	}
	default:
		break;
	}
}
