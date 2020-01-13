#include "JPGWidget.h"
#include <QPainter>
#include <QTimerEvent>

JPGWidget::JPGWidget(QWidget *parent)
	: QWidget(parent), d_timerId(-1)
{
	m_pixmap = nullptr;
	m_dCurPencent = 0.0;
	m_dIncreConst = 0.02;

	m_dIncre = m_dIncreConst;
}

JPGWidget::~JPGWidget()
{

}

void JPGWidget::start()
{
	//d_clock.start();
	d_timerId = startTimer(500);
}

void JPGWidget::stop()
{
	killTimer(d_timerId);
}

void JPGWidget::setPause(bool bPause)
{
	m_bPause = bPause;

	if (m_bPause)
	{
		m_dIncre = 0.0;
	}
	else
	{
		m_dIncre = m_dIncreConst;
	}
}

void JPGWidget::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == d_timerId)
	{
		m_dCurPencent += m_dIncre;
		if (m_dCurPencent > 1.0)
			m_dCurPencent = 0.0;

		update();
		return;
	}

	QWidget::timerEvent(event);
}

bool JPGWidget::loadJPG(QString strPath)
{
	if (m_pixmap != nullptr)
	{
		delete m_pixmap;
		m_pixmap = nullptr;
	}

	QImage* pImage = new QImage(strPath);
	if (pImage == nullptr)
		return false;

	m_pixmap = new QPixmap();
	m_pixmap->convertFromImage(*pImage);

	return true;
}

void JPGWidget::paintEvent(QPaintEvent *)
{
	if (m_pixmap == nullptr)
		return;

	int nSrcWid = m_pixmap->width();
	int nSrcHei = m_pixmap->height();

	int nDstWid = rect().width();
	int nDstHei = rect().height();

	double dScale = 1.0;
	if (nSrcWid / (double)nSrcHei <= nDstWid / (double)nDstHei)
	{
		dScale = (double)nDstHei / nSrcHei;
	}
	else
	{
		dScale = (double)nDstWid / nSrcWid;
	}

	double dx1 = rect().center().rx();
	double dy1 = rect().center().ry();

	QRectF recTarget(dx1 - nSrcWid * 0.5 * dScale, dy1 - nSrcHei * 0.5 * dScale, nSrcWid * dScale, nSrcHei * dScale * m_dCurPencent);
	QRectF recSource(0.0, 0.0, nSrcWid, nSrcHei * m_dCurPencent);

	QPainter painter(this);
	painter.drawPixmap(recTarget, *m_pixmap, recSource);
}