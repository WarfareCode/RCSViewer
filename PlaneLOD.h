#ifndef PLANELOD_H
#define PLANELOD_H

#include <osg/LOD>

class PlaneLOD : public osg::LOD
{
public:
	PlaneLOD();
	~PlaneLOD();

	virtual void traverse(osg::NodeVisitor& nv) override;

private:
	
};

#endif // PLANELOD_H
