#ifndef VIEWERWIDGET_H
#define VIEWERWIDGET_H

#include <QTimer>
#include <QApplication>
#include <QGridLayout>

#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/NodeTrackerManipulator>

#include <osgDB/ReadFile>

#include <osgQt/GraphicsWindowQt>

#include <iostream>

#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Geometry>
#include <osg/Geode>

#include <osgUtil/Optimizer>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/TerrainManipulator>

#include <osgSim/OverlayNode>
#include <osgViewer/Viewer>


class ViewerWidget : public QWidget, public osgViewer::CompositeViewer
{
public:
	ViewerWidget(osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::CompositeViewer::SingleThreaded);

	QWidget* addViewWidget(osgQt::GraphicsWindowQt* gw, osg::Node* scene, osg::Node* pNode = nullptr);

	osgQt::GraphicsWindowQt* createGraphicsWindow(int x, int y, int w, int h, const std::string& name = "", bool windowDecoration = false);

	virtual void paintEvent(QPaintEvent* event)
	{
		frame();
	}

protected:

	QTimer _timer;
};

#endif // VIEWERWIDGET_H
