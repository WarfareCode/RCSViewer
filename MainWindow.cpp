#include "MainWindow.h"
#include <osgViewer/Viewer>
#include "ViewerWidget.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowTitle(QString::fromLocal8Bit("Ä£Äâ"));

	QMenu* pMenuFile = menuBar()->addMenu(QString::fromLocal8Bit("ÎÄ¼þ"));

#if QT_VERSION >= 0x050000
	// Qt5 is currently crashing and reporting "Cannot make QOpenGLContext current in a different thread" when the viewer is run multi-threaded, this is regression from Qt4
	osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::SingleThreaded;
#else
	osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::CullDrawThreadPerContext;
#endif

	ViewerWidget* viewWidget = new ViewerWidget(threadingModel);
	setCentralWidget(viewWidget);
}

MainWindow::~MainWindow()
{

}
