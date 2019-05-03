#include"gps_rcs_files_read.h"
#include<math.h>
#include <QStringList>
#include <fstream>
#include <QVector>

void LinearIntp(double *xout, double *yout, __int64 xn, double *yin, __int64 yn)
{
	double coef = 1.0;
	long   loc = 0;
	for (long k = 0; k < xn; k++)
	{
		loc = long(xout[k]);
		coef = xout[k] - loc;
		if (loc >= yn - 1) loc = yn - 2;
		yout[k] = yin[loc] * (1 - coef) + yin[loc + 1] * coef;
	}
}

bool gps_rcs_files_read(QString gpsfile,
	QString targpsfile,
	QString rcsfile,
	QVector<dataunit*> &vec_data,
	cTime& startTime)
{
	if (vec_data.size() > 0)
	{
		vec_data.clear();
	}

	//cTime time_temp;
	//target
	double *tar_lati_raw = new double[MAX_RCS_POINTS];
	double *tar_logn_raw = new double[MAX_RCS_POINTS];
	double *tar_h_raw = new double[MAX_RCS_POINTS];
	double *tar_hx_raw = new double[MAX_RCS_POINTS];
	double *tar_sec_raw = new double[MAX_RCS_POINTS];

	memset(tar_lati_raw, 0, sizeof(double)*MAX_RCS_POINTS);
	memset(tar_logn_raw, 0, sizeof(double)*MAX_RCS_POINTS);
	memset(tar_h_raw, 0, sizeof(double)*MAX_RCS_POINTS);
	memset(tar_hx_raw, 0, sizeof(double)*MAX_RCS_POINTS);
	memset(tar_sec_raw, 0, sizeof(double)*MAX_RCS_POINTS);

	// plane
	double *plane_lati_raw = new double[MAX_RCS_POINTS];
	double *plane_logn_raw = new double[MAX_RCS_POINTS];
	double *plane_h_raw = new double[MAX_RCS_POINTS];
	memset(plane_lati_raw, 0, sizeof(double)*MAX_RCS_POINTS);
	memset(plane_logn_raw, 0, sizeof(double)*MAX_RCS_POINTS);
	memset(plane_h_raw, 0, sizeof(double)*MAX_RCS_POINTS);

	//RCS
	double* RCS_raw = new double[MAX_RCS_POINTS];
	memset(RCS_raw, 0, sizeof(double)*MAX_RCS_POINTS);

	//read plane GPS files
	char line[1024];

	QByteArray tempArray = gpsfile.toLocal8Bit();
	char* pPath = tempArray.data();
	std::ifstream fin(pPath, std::ios::in);
	long plat_gps_point = 0;

	QStringList plat_time;
	QStringList plat_date;

	while (fin.getline(line, sizeof(line)))
	{
		QString cmd = QString("%1").arg(line);
		if (cmd.isEmpty())continue;
		QStringList list;
		list = cmd.split(QRegExp("\\s+"));

		if (plat_gps_point >= MAX_RCS_POINTS) break;

		plat_time.append(list.at(1));
		plat_date.append(list.at(0));
		plane_logn_raw[plat_gps_point] = list.at(2).toDouble();
		plane_lati_raw[plat_gps_point] = list.at(3).toDouble();
		plane_h_raw[plat_gps_point] = list.at(4).toDouble();

		plat_gps_point++;
	};
	fin.close();

	if (plat_gps_point == 0)
	{
		//QMessageBox::critical(this, tr("错误"), tr("飞机GPS文件无法读取！"));
		return false;
	}
	//read target GPS files

	QStringList tar_time;
	std::ifstream fin1(targpsfile.toLocal8Bit().data(), std::ios::in);
	long tar_gps_point = 0;

	while (fin1.getline(line, sizeof(line)))
	{
		QString cmd = QString("%1").arg(line);
		if (cmd.isEmpty())continue;
		QStringList list;
		list = cmd.split(QRegExp("\\s+"));
		if (tar_gps_point >= MAX_RCS_POINTS) break;

		tar_time.append(list.at(0)); // seconds for 0:0:00
		tar_sec_raw[tar_gps_point] = list.at(0).toDouble();
		tar_logn_raw[tar_gps_point] = list.at(1).toDouble();
		tar_lati_raw[tar_gps_point] = list.at(2).toDouble();
		tar_h_raw[tar_gps_point] = list.at(3).toDouble();
		tar_hx_raw[tar_gps_point] = list.at(6).toDouble();

		tar_gps_point++;
	};
	fin1.close();

	if (tar_gps_point == 0)
	{
		//QMessageBox::critical(this, tr("错误"), tr("目标GPS文件无法读取！"));
		return false;
	}

	QStringList list_d = plat_date.at(0).split("-");
	startTime.iYear = list_d.at(0).toInt();
	startTime.iMonth = list_d.at(1).toInt();
	startTime.iDay = list_d.at(2).toInt();

	QString plat_time_s = plat_time.at(0); //[0];
	QString plat_time_e = plat_time.at(plat_gps_point - 1);

	// data cut
	QStringList list_s = plat_time_s.split(":");

	long plat_sta_sec = 0;
	plat_sta_sec = list_s.at(2).toLong() + list_s.at(1).toLong() * 60 + list_s.at(0).toLong() * 3600;

	QStringList list_e = plat_time_e.split(":");
	long plat_end_sec = 0;
	plat_end_sec = list_e.at(2).toLong() + list_e.at(1).toLong() * 60 + list_e.at(0).toLong() * 3600;;

	long tar_sta_sec = tar_time.at(0).toLong();
	long tar_end_sec = tar_time.at(tar_gps_point - 1).toLong();

	long tar_loc_sta = long(double(plat_sta_sec - tar_sta_sec)*tar_gps_point / (tar_end_sec - tar_sta_sec + 1)); // IMU update
	long tar_loc_end = long(double(plat_end_sec - tar_sta_sec)*tar_gps_point / (tar_end_sec - tar_sta_sec + 1));
	long tar_points = tar_loc_end - tar_loc_sta + 1;

	if (tar_points <= 0)
	{
		//QMessageBox::critical(this, tr("错误"), tr("无法从目标GPS文件对应载机数据！"));
		return false;
	}

	// 读取RCS文件，获取总点数
	std::ifstream fin2(rcsfile.toLocal8Bit().data(), std::ios::in);
	long rcs_points = 0;
	while (fin2.getline(line, sizeof(line))) 
	{
		QString cmd = QString("%1").arg(line);
		if (cmd.isEmpty())continue;
		QStringList list;
		list = cmd.split(QRegExp("\\s+"), QString::SkipEmptyParts);
		RCS_raw[rcs_points] = list.at(2).toDouble();
		if (rcs_points >= MAX_RCS_POINTS) break;

		rcs_points++;
	}
	fin2.close();

	if (rcs_points <= 0)
	{
		//QMessageBox::critical(this, tr("错误"), tr("无法从RCS文件读取数据！"));
		return false;
	}

	// all above is ok
	// 插值tar_points,plat_gps_point,rcs_points个点数
	long N = rcs_points;
	if ((tar_points > plat_gps_point) && (tar_points > N))	  N = tar_points;
	if ((plat_gps_point > tar_points) && (plat_gps_point > N)) N = plat_gps_point;

	// target
	double *tar_lati_intp = new double[N];
	double *tar_logn_intp = new double[N];
	double *tar_h_intp = new double[N];
	double *tar_hx_intp = new double[N];
	double *tar_sec_intp = new double[N];

	memset(tar_lati_intp, 0, sizeof(double)*N);
	memset(tar_logn_intp, 0, sizeof(double)*N);
	memset(tar_h_intp, 0, sizeof(double)*N);
	memset(tar_hx_intp, 0, sizeof(double)*N);
	memset(tar_sec_intp, 0, sizeof(double)*N);

	// plane
	double *plane_lati_intp = new double[N];
	double *plane_logn_intp = new double[N];
	double *plane_h_intp = new double[N];
	memset(plane_lati_intp, 0, sizeof(double)*N);
	memset(plane_logn_intp, 0, sizeof(double)*N);
	memset(plane_h_intp, 0, sizeof(double)*N);

	double *intp_x_tar = new double[rcs_points];
	double *intp_x_plane = new double[rcs_points];
	for (long k = 0; k < rcs_points; k++)  
		intp_x_tar[k] = tar_points*(double)k / rcs_points;

	for (long k = 0; k < rcs_points; k++)  
		intp_x_plane[k] = plat_gps_point*(double)k / rcs_points;

	// interp for target gps data
	LinearIntp(intp_x_tar, tar_lati_intp, rcs_points, tar_lati_raw + tar_loc_sta, tar_points);
	LinearIntp(intp_x_tar, tar_logn_intp, rcs_points, tar_logn_raw + tar_loc_sta, tar_points);
	LinearIntp(intp_x_tar, tar_h_intp, rcs_points, tar_h_raw + tar_loc_sta, tar_points);
	LinearIntp(intp_x_tar, tar_hx_intp, rcs_points, tar_hx_raw + tar_loc_sta, tar_points);
	LinearIntp(intp_x_tar, tar_sec_intp, rcs_points, tar_sec_raw + tar_loc_sta, tar_points);

	// interp for plane gps data
	LinearIntp(intp_x_plane, plane_lati_intp, rcs_points, plane_lati_raw, plat_gps_point);
	LinearIntp(intp_x_plane, plane_logn_intp, rcs_points, plane_logn_raw, plat_gps_point);
	LinearIntp(intp_x_plane, plane_h_intp, rcs_points, plane_h_raw, plat_gps_point);

	// 计算方位角并保存；
	double *azAngle = new double[rcs_points];
	double *ptAngle = new double[rcs_points];
	double *sltRange = new double[rcs_points];
	memset(azAngle, 0, sizeof(double)*rcs_points);
	memset(ptAngle, 0, sizeof(double)*rcs_points);
	memset(sltRange, 0, sizeof(double)*rcs_points);

	int hour_temp, min_temp;
	for (long k = 0; k < rcs_points; k++)
	{
		double X_lati = (plane_lati_intp[k] - tar_lati_intp[k])*111693.1;  // 纬度信息地面投影
		double Y_logn = (plane_logn_intp[k] - tar_logn_intp[k])*111693.1*cos(plane_lati_intp[k] * pi / 180.0); //经度信息地面投影

		double high = (plane_h_intp[k] - tar_h_intp[k]); // 载机到地面的高度
		sltRange[k] = sqrt(high*high + X_lati*X_lati + Y_logn*Y_logn); // 载机到目标的斜距

		ptAngle[k] = 90 - acos(high / sltRange[k]) * 180 / pi; // 与水平面的夹角为俯仰角;
		azAngle[k] = atan2(Y_logn, X_lati) * 180 / pi;           // 与正北方向的夹角为定义的方位角
		if (azAngle[k] < 0) azAngle[k] += 360;

		azAngle[k] -= tar_hx_intp[k];  // 计算相对于船的航向的夹角；

		while (azAngle[k] < 0)	{ azAngle[k] += 360; }
		while (azAngle[k] > 360) { azAngle[k] -= 360; } // 添加2017/3/25;

		dataunit* pUnit = new dataunit;
		dataunit& dataunit_temp = *pUnit;

		dataunit_temp.angle = azAngle[k];
		dataunit_temp.RCS_dB = RCS_raw[k];
		dataunit_temp.plane_lon = plane_logn_intp[k];
		dataunit_temp.plane_lat = plane_lati_intp[k];
		dataunit_temp.plane_Height = plane_h_intp[k];
		dataunit_temp.target_lon = tar_logn_intp[k];
		dataunit_temp.target_lat = tar_lati_intp[k];
		dataunit_temp.target_Height = tar_h_intp[k];

// 		dataunit_temp.ctime.iYear = time_temp.iYear;
// 		dataunit_temp.ctime.iMonth = time_temp.iMonth;
// 		dataunit_temp.ctime.iDay = time_temp.iDay;

// 		hour_temp = floor(tar_sec_intp[k] / 3600);
// 
// 		dataunit_temp.ctime.dHours = hour_temp;
// 		min_temp = floor((tar_sec_intp[k] - 3600 * hour_temp) / 60);
// 		dataunit_temp.ctime.dMinutes = min_temp;
// 		dataunit_temp.ctime.dSeconds = floor(tar_sec_intp[k] - 3600 * hour_temp - 60 * min_temp);

		dataunit_temp.dTime = tar_sec_intp[k];
		vec_data.push_back(pUnit);
	}

	delete[] tar_lati_raw; tar_lati_raw = NULL;
	delete[] tar_logn_raw; tar_logn_raw = NULL;
	delete[] tar_h_raw;    tar_h_raw = NULL;
	delete[] tar_hx_raw;   tar_hx_raw = NULL;
	delete[] tar_sec_raw;  tar_sec_raw = NULL;

	delete[] tar_lati_intp; tar_lati_intp = NULL;
	delete[] tar_logn_intp; tar_logn_intp = NULL;
	delete[] tar_hx_intp;	 tar_hx_intp = NULL;
	delete[] tar_sec_intp;  tar_sec_intp = NULL;

	delete[] plane_lati_raw; plane_lati_raw = NULL;
	delete[] plane_logn_raw; plane_logn_raw = NULL;
	delete[] plane_h_raw;    plane_h_raw = NULL;
	delete[] RCS_raw;   RCS_raw = NULL;

	delete[] plane_lati_intp; plane_lati_intp = NULL;
	delete[] plane_logn_intp; plane_logn_intp = NULL;

	delete[] intp_x_tar; intp_x_tar = NULL;
	delete[] intp_x_plane; intp_x_plane = NULL;

	delete[] azAngle; azAngle = NULL;
	delete[] ptAngle; ptAngle = NULL;

	delete[] plane_h_intp;    plane_h_intp = NULL;
	delete[] tar_h_intp;	   tar_h_intp = NULL;
	delete[] sltRange;        sltRange = NULL;

	return true;
}
