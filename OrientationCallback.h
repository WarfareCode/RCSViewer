#ifndef ORIENTATIONCALLBACK_H
#define ORIENTATIONCALLBACK_H

#include <osg/NodeCallback>
#include <osg/Array>
#include <osg/NodeVisitor>
#include "DataManager.h"

class OrientationCallback : public osg::NodeCallback
{
public:

	OrientationCallback();
	~OrientationCallback();

	void Clear()
	{
		m_pArray->clear();
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

protected:

	double m_dLastTime;
	double m_dLastTime2;

	int m_nState;
	int m_dLastSize;
	double m_dTimeInterval;
	double m_dInterValTemp;

	osg::ref_ptr<osg::Vec3Array> m_pArray;

};

#endif // ORIENTATIONCALLBACK_H
