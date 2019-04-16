#ifndef RADARBEAMCALLBACK_H
#define RADARBEAMCALLBACK_H

#include <osg/NodeCallback>

class RadarBeamCallback : public osg::NodeCallback
{
public:
	RadarBeamCallback();
	~RadarBeamCallback();

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

private:

	double m_dLastTime;
	
};

#endif // RADARBEAMCALLBACK_H
