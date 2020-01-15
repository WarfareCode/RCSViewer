#include "plot.h"
#include "curvedata.h"
#include "signaldata.h"

#include <QCoreApplication>
#include <QMouseEvent>

#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_div.h>
#include <qwt_scale_map.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_painter.h>
#include <qwt_text.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include "qwt_symbol.h"
#include <qcoreevent.h>
#include "PlotSettings.h"

class Canvas : public QwtPlotCanvas
{
public:
	Canvas(QwtPlot *plot = NULL) :
		QwtPlotCanvas(plot)
	{
		// The backing store is important, when working with widget
		// overlays ( f.e rubberbands for zooming ).
		// Here we don't have them and the internal
		// backing store of QWidget is good enough.

		setPaintAttribute(QwtPlotCanvas::BackingStore, false);
		setBorderRadius(10);

		if (QwtPainter::isX11GraphicsSystem())
		{
#if QT_VERSION < 0x050000
			// Even if not liked by the Qt development, Qt::WA_PaintOutsidePaintEvent
			// works on X11. This has a nice effect on the performance.

			setAttribute( Qt::WA_PaintOutsidePaintEvent, true );
#endif

			// Disabling the backing store of Qt improves the performance
			// for the direct painter even more, but the canvas becomes
			// a native window of the window system, receiving paint events
			// for resize and expose operations. Those might be expensive
			// when there are many points and the backing store of
			// the canvas is disabled. So in this application
			// we better don't disable both backing stores.

			if (testPaintAttribute(QwtPlotCanvas::BackingStore))
			{
				setAttribute(Qt::WA_PaintOnScreen, true);
				setAttribute(Qt::WA_NoSystemBackground, true);
			}
		}

		setupPalette();
	}

private:
	void setupPalette()
	{
		QPalette pal = palette();

#if QT_VERSION >= 0x040400
		QLinearGradient gradient;
		gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
		gradient.setColorAt(0.0, QColor(0, 49, 110));
		gradient.setColorAt(1.0, QColor(0, 87, 174));

		pal.setBrush(QPalette::Window, QBrush(gradient));
#else
		pal.setBrush( QPalette::Window, QBrush( color ) );
#endif

		// QPalette::WindowText is used for the curve color
		pal.setColor(QPalette::WindowText, Qt::green);

		setPalette(pal);
	}
};

class MyPlotPicker : public QwtPlotPicker
{
public:

	explicit MyPlotPicker(QWidget *canvas) : QwtPlotPicker(canvas)
	{

	}

	~MyPlotPicker(){}

	//改为显示一位小数
	QwtText trackerTextF(const QPointF &pos) const override
	{
		QString text;

		switch (rubberBand())
		{
		case HLineRubberBand:
			text.sprintf("%.1f", pos.y());
			break;
		case VLineRubberBand:
			text.sprintf("%.1f", pos.x());
			break;
		default:
			text.sprintf("%.1f, %.1f", pos.x(), pos.y());
		}
		return QwtText(text);
	}
};

Plot::Plot(QWidget *parent) :
QwtPlot(parent),
d_paintedPoints(0),
d_interval(0.0, 360.0),
d_timerId(-1)
{
	d_directPainter = new QwtPlotDirectPainter();

	PlotSettings* pPlotSettings = PlotSettings::Instance();

	setAutoReplot(false);
	setCanvas(new Canvas());

	plotLayout()->setAlignCanvasToScales(true);

	int nHMin, nHMax, nVMin, nVMax;
	PlotSettings* pSettings = PlotSettings::Instance();
	pSettings->getHMinMax(nHMin, nHMax);
	pSettings->getVMinMax(nVMin, nVMax);

	setAxisTitle(QwtPlot::xBottom, pPlotSettings->hLabel());
	setAxisTitle(QwtPlot::yLeft, pPlotSettings->vLabel());

	setAxisScale(QwtPlot::xBottom, nHMin, nHMax);
	setAxisScale(QwtPlot::yLeft, nVMin, nVMax);

	QwtPlotGrid *grid = new QwtPlotGrid();
	grid->setPen(Qt::gray, 0.0, Qt::DotLine);
	grid->enableX(true);
	grid->enableXMin(true);
	grid->enableY(true);
	grid->enableYMin(false);
	grid->attach(this);

// 	d_origin = new QwtPlotMarker();
// 	d_origin->setLineStyle(QwtPlotMarker::Cross);
// 	d_origin->setValue(d_interval.minValue() + d_interval.width() / 2.0, 30/*0.0*/);
// 	d_origin->setLinePen(Qt::gray, 0.0, Qt::DashLine);
// 	d_origin->attach(this);

	d_curve = new QwtPlotCurve();
	d_curve->setStyle(QwtPlotCurve::Lines);
	d_curve->setPen(canvas()->palette().color(QPalette::WindowText));
	d_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	d_curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);

	// 	d_curve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::yellow), QPen(Qt::red, 2), QSize(4, 4)));
	// 	d_curve->setPen(QColor(Qt::darkGreen));
	// 	d_curve->setStyle(QwtPlotCurve::Dots);
	//d_curve->setCurveAttribute(QwtPlotCurve::Fitted);

	d_curve->setData(new CurveData());
	d_curve->attach(this);

	// panning with the left mouse button
	(void)new QwtPlotPanner(canvas());

	// zoom in/out with the wheel
	QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(canvas());
	magnifier->setMouseButton(Qt::NoButton);

	if (1)
	{
		auto picker = new MyPlotPicker(canvas());
		picker->setStateMachine(new QwtPickerDragPointMachine());
		picker->setRubberBandPen(QColor(Qt::darkMagenta));
		picker->setRubberBand(QwtPicker::CrossRubberBand);
		picker->setTrackerMode(QwtPicker::AlwaysOn);//被激活时候显示
		picker->setTrackerPen(QColor(Qt::red));//显示实时的坐标
		QFont font = picker->trackerFont();
		font.setPixelSize(24);
		font.setBold(true);
		picker->setTrackerFont(font);

		//QwtPlotPicker* picker = new QwtPlotPicker(QwtPlot::xBottom, 10,
		//	QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
		//	canvas());
		//picker->setStateMachine(new QwtPickerDragPointMachine());
		//picker->setRubberBandPen(QColor(Qt::darkMagenta));
		//picker->setRubberBand(QwtPicker::CrossRubberBand);
		//picker->setTrackerPen(QColor(Qt::green));
	}
}

Plot::~Plot()
{
	delete d_directPainter;
}

void Plot::start()
{
	//d_clock.start();
	d_timerId = startTimer(100);
}

void Plot::stop()
{
	killTimer(d_timerId);
}

void Plot::replot()
{
	CurveData *curveData = static_cast<CurveData *>(d_curve->data());
	curveData->values().lock();

	QwtPlot::replot();
	d_paintedPoints = curveData->size();

	curveData->values().unlock();
}

void Plot::setIntervalLength(double interval)
{
	if (interval > 0.0 && interval != d_interval.width())
	{
		d_interval.setMaxValue(d_interval.minValue() + interval);
		setAxisScale(QwtPlot::xBottom,
			d_interval.minValue(), d_interval.maxValue());

		replot();
	}
}

void Plot::updateCurve()
{
	CurveData *curveData = static_cast<CurveData *>(d_curve->data());
	curveData->values().lock();

	const int numPoints = curveData->size();
	if (numPoints > d_paintedPoints)
	{
		const bool doClip = !canvas()->testAttribute(Qt::WA_PaintOnScreen);
		if (doClip)
		{
			/*
				Depending on the platform setting a clip might be an important
				performance issue. F.e. for Qt Embedded this reduces the
				part of the backing store that has to be copied out - maybe
				to an unaccelerated frame buffer device.
				*/

			const QwtScaleMap xMap = canvasMap(d_curve->xAxis());
			const QwtScaleMap yMap = canvasMap(d_curve->yAxis());

			QRectF br = qwtBoundingRect(*curveData,
				d_paintedPoints - 1, numPoints - 1);

			const QRect clipRect = QwtScaleMap::transform(xMap, yMap, br).toRect();
			d_directPainter->setClipRegion(clipRect);
		}

		d_directPainter->drawSeries(d_curve,
			d_paintedPoints - 1, numPoints - 1);
		d_paintedPoints = numPoints;
	}

	curveData->values().unlock();
}

void Plot::incrementInterval()
{
	d_interval = QwtInterval(d_interval.maxValue(),
		d_interval.maxValue() + d_interval.width());

	CurveData *curveData = static_cast<CurveData *>(d_curve->data());
	curveData->values().clearStaleValues(d_interval.minValue());

	// To avoid, that the grid is jumping, we disable
	// the autocalculation of the ticks and shift them
	// manually instead.

	QwtScaleDiv scaleDiv = axisScaleDiv(QwtPlot::xBottom);
	scaleDiv.setInterval(d_interval);

	for (int i = 0; i < QwtScaleDiv::NTickTypes; i++)
	{
		QList<double> ticks = scaleDiv.ticks(i);
		for (int j = 0; j < ticks.size(); j++)
			ticks[j] += d_interval.width();
		scaleDiv.setTicks(i, ticks);
	}
	setAxisScaleDiv(QwtPlot::xBottom, scaleDiv);

	d_origin->setValue(d_interval.minValue() + d_interval.width() / 2.0, 0.0);

	d_paintedPoints = 0;
	replot();
}

void Plot::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == d_timerId)
	{
		//ln_debug 20190826 例子代码写的是updateCurve，但是加入QwtPlotPicker后会有残留
		// 改为replot。速度应该是降低了，但是没有残留。
		//updateCurve();
		replot();


// 		const double elapsed = d_clock.elapsed() / 1000.0;
// 		if (elapsed > d_interval.maxValue())
// 			incrementInterval();

		return;
	}

	QwtPlot::timerEvent(event);
}

void Plot::resizeEvent(QResizeEvent *event)
{
	d_directPainter->reset();
	QwtPlot::resizeEvent(event);
}

void Plot::showEvent(QShowEvent *)
{
	replot();
}

bool Plot::eventFilter(QObject *object, QEvent *event)
{
	if (object == canvas() &&
		event->type() == QEvent::PaletteChange)
	{
		d_curve->setPen(canvas()->palette().color(QPalette::WindowText));
	}

	return QwtPlot::eventFilter(object, event);
}
