#ifndef JPGWIDGET_H
#define JPGWIDGET_H

#include <QWidget>

class JPGWidget : public QWidget
{
	Q_OBJECT

public:
	JPGWidget(QWidget *parent = 0);
	~JPGWidget();

	bool loadJPG(QString strPath);

	void start();
	void stop();

	void setPause(bool bPause);

protected:

	virtual void timerEvent(QTimerEvent *event);
	virtual void paintEvent(QPaintEvent *) override;

private:

	QPixmap* m_pixmap;

	double m_dCurPencent;

	double m_dIncre;

	double m_dIncreConst;

	int d_timerId;

	bool m_bPause;
};

#endif // JPGWIDGET_H
