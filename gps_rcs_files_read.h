#ifndef GPS_RCS_FILES_READ_H
#define GPS_RCS_FILES_READ_H

#include<vector>
using namespace std;
#include <QtCore/QString>
#include<QVector>

#define MAX_RCS_POINTS 500000
#define MAX_AZI_POINTS 500000
//#define C 3.0e8

#define MAX_FILES 100
#define pi 3.14159265


typedef struct cTime{
	int iYear;
	int iMonth;
	int iDay;
// 	double dHours;
// 	double dMinutes;
// 	double dSeconds;

}cTime;

typedef  struct dataunit
{
	double angle;
	double RCS_dB;
	double plane_lat;
	double plane_lon;
	double plane_Height;
	double target_lat;
	double target_lon;
	double target_Height;
	//cTime  ctime;
	double dTime;
}detaunit;


int FindAngleIndex(double *Anlge,double angle,int Npoints);
void LinearIntp(double *xout, double *yout, __int64 xn, double *yin, __int64 yn);

bool gps_rcs_files_read(QString gpsfile,
	QString targpsfile,
	QString rcsfile,
	QVector<dataunit> &vec_data, 
	cTime& startTime);

    bool gps_rcs_files_read_single(QString gpsfile,
	double tar_lon,
	double tar_lat,
	double tar_h,
	double tar_hx,
	QString rcsfile,
	QVector<dataunit> &vec_data, 
	cTime& startTime);

	bool gps_rcs_files_read_Ex(QString gpsfile,
		QString targpsfile,
		QString rcsfile,
		QVector<dataunit> &vec_data,
		cTime& startTime);

#endif // GPS_RCS_FILES_READ_H
