#ifndef SAMPLING_THREAD
#define SAMPLING_THREAD

#include <qwt_sampling_thread.h>
#include <QtCore/QSemaphore>

class SamplingThread: public QwtSamplingThread
{
    Q_OBJECT

public:
    SamplingThread( QObject *parent = NULL );

    double frequency() const;
    double amplitude() const;

	void Cancel();

public Q_SLOTS:
    void setAmplitude( double );
    void setFrequency( double );

protected:
    virtual void sample( double elapsed ) QWT_OVERRIDE;

	virtual void run() QWT_OVERRIDE;

private:
    virtual double value( double timeStamp ) const;

    double d_frequency;
    double d_amplitude;

	QSemaphore m_semaphore;
};

#endif
