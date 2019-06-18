#ifndef SIGNAL_DATA_H
#define SIGNAL_DATA_H

#include <qrect.h>

typedef struct
{
	double dTime;
	double angle;
	double RCS_dB;
}RCSRecord;

class SignalData
{
public:
    static SignalData &instance();

    void append( const QPointF &pos );
    void clearStaleValues( double min );

    int size() const;
    QPointF value( int index ) const;

    QRectF boundingRect() const;

	void Clear();

    void lock();
    void unlock();

private:
    SignalData();
    SignalData( const SignalData & );
    SignalData &operator=( const SignalData & );

    virtual ~SignalData();

    class PrivateData;
    PrivateData *d_data;
};

#endif
