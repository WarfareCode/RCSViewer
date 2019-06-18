#include "samplingthread.h"
#include "signaldata.h"

#include <qwt_math.h>
#include <qmath.h>
#include <QVector>
#include <QtCore/QMap>
#include <QMutexLocker>
#include "gps_rcs_files_read.h"
#include "plot.h"

#if QT_VERSION < 0x040600
#define qFastSin(x) ::sin(x)
#endif

//extern QVector<dataunit>  g_vecData;
extern QMutex g_MutexData;
extern QVector<RCSRecord> g_vecRCSRecord;

extern Plot* g_pPlot;

SamplingThread::SamplingThread( QObject *parent ):
    QwtSamplingThread( parent ),
    d_frequency( 5.0 ),
    d_amplitude( 20.0 ),
	m_semaphore(1)
{
}

void SamplingThread::setFrequency( double frequency )
{
    d_frequency = frequency;
}

double SamplingThread::frequency() const
{
    return d_frequency;
}

void SamplingThread::setAmplitude( double amplitude )
{
    d_amplitude = amplitude;
}

double SamplingThread::amplitude() const
{
    return d_amplitude;
}

void SamplingThread::sample( double elapsed )
{
    if ( d_frequency > 0.0 )
    {
		QMutexLocker locker(&g_MutexData);
       // const QPointF s( elapsed, value( elapsed ) );

		if (g_vecRCSRecord.isEmpty())
			return;

		double dTimeTotal = g_vecRCSRecord.last().dTime;
		double dTemp = elapsed / dTimeTotal;
		dTemp = dTemp - (int)dTemp;
		double dTime = dTemp* dTimeTotal;

		static int s_nIndex = 0;

		int nIndex = 0;
		for (int i = 0; i < g_vecRCSRecord.size(); i ++)
		{
			RCSRecord& record = g_vecRCSRecord[i];
			if (record.dTime > dTime)
			{
				nIndex = i;
				break;
			}
		}

		if (nIndex >= s_nIndex)
		{
			for (int i = s_nIndex; i < nIndex; i ++)
			{
				RCSRecord& record = g_vecRCSRecord[i];

				QPointF s(record.angle, record.RCS_dB);
				SignalData::instance().append(s);
			}
		}
		else
		{
			SignalData::instance().Clear();

			for (int i = 0; i < nIndex; i ++)
			{
				RCSRecord& record = g_vecRCSRecord[i];

				QPointF s(record.angle, record.RCS_dB);
				SignalData::instance().append(s);
			}

			g_pPlot->replot();
		}

		s_nIndex = nIndex;
    }
}

void SamplingThread::run()
{
	m_semaphore.acquire();
	QwtSamplingThread::run();
	m_semaphore.release();
}

void SamplingThread::Cancel()
{
	QwtSamplingThread::stop();

	m_semaphore.acquire();
	m_semaphore.release();
}

double SamplingThread::value( double timeStamp ) const
{
    const double period = 1.0 / d_frequency;

    const double x = std::fmod( timeStamp, period );
    const double v = d_amplitude * qFastSin( x / period * 2 * M_PI );

    return v;
}
