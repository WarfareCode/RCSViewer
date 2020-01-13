#include"gps_rcs_files_read.h"
#include<math.h>
#include <QStringList>
#include <fstream>
#include <QVector>

int FindAngleIndex(double *Anlge,double angle,int Npoints)
{

     int index;
    if(angle<Anlge[0])
    {
        index=0;
        return  index;

    }

    if(angle>Anlge[Npoints-1])
    {
        index=Npoints-1;
        return index;
    }

    for(int ii=0;ii<Npoints-1;ii++)
    {
        if(angle>=Anlge[ii]&&angle<Anlge[ii+1])
        {
            if(abs(angle-Anlge[ii])<abs(angle-Anlge[ii+1]))
            {
                index=ii;
            }
            else
            {
                index=ii+1;
            }
        }
    }

    return index;
}

void LinearIntp(double *xout, double *yout, __int64 xn, double *yin, __int64 yn)
{
	double coef = 1.0;
	long   loc = 0;
	for (long k = 0; k < xn; k++)
	{
        loc = floor(xout[k]);
		coef = xout[k] - loc;
		if (loc >= yn - 1) loc = yn - 2;
		yout[k] = yin[loc] * (1 - coef) + yin[loc + 1] * coef;
	}
}

bool  gps_rcs_files_read(QString gpsfile,
	QString targpsfile,
	QString rcsfile,
	QVector<dataunit> &vec_data,
    cTime& startTime,
   double &  target_amuzh_angle)
{
	if (vec_data.size() > 0)
	{
		vec_data.clear();
	}

	dataunit dataunit_temp;
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

    double*RCS_raw_angle=new double[MAX_RCS_POINTS];
    memset(RCS_raw_angle,0,sizeof(double)*MAX_RCS_POINTS);

	//read plane GPS files  ====读取载机的GPS数据
	char line[1024];

	QByteArray tempArray = gpsfile.toLocal8Bit();
	char* pPath = tempArray.data();
	std::ifstream fin(pPath, std::ios::in);
	long plat_gps_point = 0;

	QStringList plat_time;
	QStringList plat_date;

   // QString time_temp;
    double  longi_temp,lati_temp,height_temp;




	while (fin.getline(line, sizeof(line)))
	{
		QString cmd = QString("%1").arg(line);
		if (cmd.isEmpty())continue;
		QStringList list;
		list = cmd.split(QRegExp("\\s+"));

		if (plat_gps_point >= MAX_RCS_POINTS) break;



        if(0==plat_gps_point)
        {
             longi_temp=list.at(2).toDouble();
             lati_temp= list.at(3).toDouble();
             height_temp=list.at(4).toDouble();

             plane_logn_raw[plat_gps_point] = list.at(2).toDouble();
             plane_lati_raw[plat_gps_point] = list.at(3).toDouble();
             plane_h_raw[plat_gps_point] = list.at(4).toDouble();
             plat_gps_point++;
             plat_time.append(list.at(1));
             plat_date.append(list.at(0));
        }



        if(list.at(2).toDouble()!=longi_temp||list.at(3).toDouble()!=lati_temp)
       {
            plane_logn_raw[plat_gps_point] = list.at(2).toDouble();
            plane_lati_raw[plat_gps_point] = list.at(3).toDouble();
            plane_h_raw[plat_gps_point] = list.at(4).toDouble();
            plat_gps_point++;
            plat_time.append(list.at(1));
            plat_date.append(list.at(0));



            longi_temp=list.at(2).toDouble();
            lati_temp= list.at(3).toDouble();
            height_temp=list.at(4).toDouble();

        }

	};
	fin.close();

	if (plat_gps_point == 0)
	{
		//QMessageBox::critical(this, tr("错误"), tr("飞机GPS文件无法读取！"));
        return false;
	}
	//read target GPS files   ====读取目标的GPS数据

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



        if(0==tar_gps_point)
        {
             longi_temp=list.at(1).toDouble();
             lati_temp= list.at(2).toDouble();
             height_temp=list.at(3).toDouble();



             tar_time.append(list.at(0)); // seconds for 0:0:00
             tar_sec_raw[tar_gps_point] = list.at(0).toDouble();
             tar_logn_raw[tar_gps_point] = list.at(1).toDouble();
             tar_lati_raw[tar_gps_point] = list.at(2).toDouble();
             tar_h_raw[tar_gps_point] = list.at(3).toDouble();
             tar_hx_raw[tar_gps_point] = list.at(6).toDouble();
             tar_gps_point++;
        }



        if(list.at(1).toDouble()!=longi_temp||list.at(2).toDouble()!=lati_temp)
       {
            tar_time.append(list.at(0)); // seconds for 0:0:00
            tar_sec_raw[tar_gps_point] = list.at(0).toDouble();
            tar_logn_raw[tar_gps_point] = list.at(1).toDouble();
            tar_lati_raw[tar_gps_point] = list.at(2).toDouble();
            tar_h_raw[tar_gps_point] = list.at(3).toDouble();
            tar_hx_raw[tar_gps_point] = list.at(6).toDouble();
            tar_gps_point++;

            longi_temp=list.at(1).toDouble();
            lati_temp= list.at(2).toDouble();
            height_temp=list.at(3).toDouble();
        }


		//第一列 秒；第二列 经度；第三列 纬度 ；第四列 高度；第七列高度    

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

    //载机起始截止相对目标的位置
    long tar_loc_sta = long(double(plat_sta_sec - tar_sta_sec)*tar_gps_point / (tar_end_sec - tar_sta_sec + 1)); // IMU update
    long tar_loc_end = long(double(plat_end_sec - tar_sta_sec)*tar_gps_point / (tar_end_sec - tar_sta_sec + 1));

    //目标起始截止相对于载机的位置
    long plat_loc_sta=long(double(tar_sta_sec-plat_sta_sec )*(plat_gps_point-1) / (plat_end_sec - plat_sta_sec ));
    long plat_loc_end=long(double(tar_end_sec - plat_sta_sec)*(plat_gps_point-1) / (plat_end_sec - plat_sta_sec ));



    double starttime_sec=plat_sta_sec;
    double endtime_sec=plat_end_sec;

// 相交计算------------------
    if(plat_sta_sec<tar_sta_sec)
    {
        tar_loc_sta=0;
        starttime_sec=tar_sta_sec;

    }
    else
    {
        plat_loc_sta=0;

    }


    if(plat_end_sec>tar_end_sec)
    {
        tar_loc_end=tar_gps_point-1;
        endtime_sec=tar_end_sec;
    }
    else
    {
        plat_loc_end=plat_gps_point-1;

    }

//实际目标点的数目
    long tar_points = tar_loc_end - tar_loc_sta + 1;
//实际载机点的数目
    long plat_points=plat_loc_end-plat_loc_sta+1;




	if (tar_points <= 0)
	{
		//QMessageBox::critical(this, tr("错误"), tr("无法从目标GPS文件对应载机数据！"));
		return false;
	}

	bool bSAR = false;
	long rcs_points = 0;
	double RCS_max = -9999.0;
	double RCS_max_angle = 0;
	if (rcsfile.endsWith(".jpg", Qt::CaseInsensitive))
		bSAR = true;
	
	if (!bSAR)
	{
		// 读取RCS文件，获取总点数
		std::ifstream fin2(rcsfile.toLocal8Bit().data(), std::ios::in);

		while (fin2.getline(line, sizeof(line)))
		{
			QString cmd = QString("%1").arg(line);
			if (cmd.isEmpty())continue;
			QStringList list;
			list = cmd.split(QRegExp("\\s+"), QString::SkipEmptyParts);
			RCS_raw[rcs_points] = list.at(1).toDouble();

			RCS_raw_angle[rcs_points] = list.at(0).toDouble();


			if (RCS_max < RCS_raw[rcs_points])
			{
				RCS_max = RCS_raw[rcs_points];
				RCS_max_angle = RCS_raw_angle[rcs_points];
			}

			if (rcs_points >= MAX_RCS_POINTS) break;
			rcs_points++;
		}
		fin2.close();

		if (rcs_points <= 0)
		{
			//QMessageBox::critical(this, tr("错误"), tr("无法从RCS文件读取数据！"));
			return false;
		}
	}



	// all above is ok
	// 插值tar_points,plat_gps_point,rcs_points个点数
    //long N = rcs_points;
    long N=0;
    //if ((tar_points > plat_gps_point) && (tar_points > N))	  N = tar_points;
    //if ((plat_gps_point > tar_points) && (plat_gps_point > N)) N = plat_gps_point;


    if(tar_points>plat_points)

        N=tar_points;
    else
        N=plat_points;

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




    double *intp_x_tar = new double[N];

    int x_temp_num=0;
    if(tar_points>plat_points)
        x_temp_num=plat_points;
    else
        x_temp_num=tar_points;
	
    for (long k = 0; k < N; k++)
    {
        intp_x_tar[k] = (x_temp_num-1)*(double)k / (N-1);
        tar_sec_intp[k]=starttime_sec+k*(endtime_sec-starttime_sec)/(N-1);
    }



	// interp for target gps data

    if(tar_points>plat_points)
    {

        for (long k = 0; k < N; k++)
        {
            tar_lati_intp[k] =*(tar_lati_raw+k+tar_loc_sta);
             tar_logn_intp[k]=*(tar_logn_raw+k+tar_loc_sta);
              tar_h_intp[k]=*(tar_h_raw+k+tar_loc_sta);
              tar_hx_intp[k]=*(tar_hx_raw+k+tar_loc_sta);
        }

        // interp for plane gps data
        LinearIntp(intp_x_tar, plane_lati_intp, N, plane_lati_raw+plat_loc_sta, plat_points);
        LinearIntp(intp_x_tar, plane_logn_intp, N, plane_logn_raw+plat_loc_sta, plat_points);
        LinearIntp(intp_x_tar, plane_h_intp, N, plane_h_raw+plat_loc_sta, plat_points);

    }
    else
    {
        LinearIntp(intp_x_tar, tar_lati_intp, N, tar_lati_raw + tar_loc_sta, tar_points);
        LinearIntp(intp_x_tar, tar_logn_intp, N, tar_logn_raw + tar_loc_sta, tar_points);
        LinearIntp(intp_x_tar, tar_h_intp, N, tar_h_raw + tar_loc_sta, tar_points);
        LinearIntp(intp_x_tar, tar_hx_intp, N, tar_hx_raw + tar_loc_sta, tar_points);
        for (long k = 0; k < N; k++)
        {
            plane_lati_intp[k] =*(plane_lati_raw+k+plat_loc_sta);
             plane_logn_intp[k]=*(plane_logn_raw+k+plat_loc_sta);
              plane_h_intp[k]=*(plane_h_raw+k+plat_loc_sta);

        }

        // interp for plane gps data

    }


	// 计算方位角并保存；
    double *azAngle = new double[N];
    double *ptAngle = new double[N];
    double *sltRange = new double[N];
    memset(azAngle, 0, sizeof(double)*N);
    memset(ptAngle, 0, sizeof(double)*N);
    memset(sltRange, 0, sizeof(double)*N);

    int RCS_index;

    for (long k = 0; k < N; k++)
	{
		double X_lati = (plane_lati_intp[k] - tar_lati_intp[k])*111693.1;  // 纬度信息地面投影
		double Y_logn = (plane_logn_intp[k] - tar_logn_intp[k])*111693.1*cos(plane_lati_intp[k] * pi / 180.0); //经度信息地面投影

		double high = (plane_h_intp[k] - tar_h_intp[k]); // 载机到地面的高度
		sltRange[k] = sqrt(high*high + X_lati*X_lati + Y_logn*Y_logn); // 载机到目标的斜距

		ptAngle[k] = 90 - acos(high / sltRange[k]) * 180 / pi; // 与水平面的夹角为俯仰角;
		azAngle[k] = atan2(Y_logn, X_lati) * 180 / pi;           // 与正北方向的夹角为定义的方位角
		if (azAngle[k] < 0) azAngle[k] += 360;

        //azAngle[k] -= tar_hx_intp[k];  // 计算相对于船的航向的夹角；

		while (azAngle[k] < 0)	{ azAngle[k] += 360; }
		while (azAngle[k] > 360) { azAngle[k] -= 360; } // 添加2017/3/25;



		dataunit_temp.angle = azAngle[k];

		if (!bSAR)
		{
			RCS_index = FindAngleIndex(RCS_raw_angle, dataunit_temp.angle, rcs_points);
			dataunit_temp.RCS_dB = RCS_raw[RCS_index];
		}
		else
		{
			dataunit_temp.RCS_dB = 0.0;
		}

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

		vec_data.push_back(dataunit_temp);
	}


   target_amuzh_angle=90-RCS_max_angle;
   while(target_amuzh_angle<0)
       target_amuzh_angle=target_amuzh_angle+360;
   while(target_amuzh_angle>360)
       target_amuzh_angle=target_amuzh_angle-360;



	delete[] tar_lati_intp; tar_lati_intp = NULL;
	delete[] tar_logn_intp; tar_logn_intp = NULL;
	delete[] tar_hx_intp;	 tar_hx_intp = NULL;
	delete[] tar_sec_intp;  tar_sec_intp = NULL;

	delete[] plane_lati_raw; plane_lati_raw = NULL;
	delete[] plane_logn_raw; plane_logn_raw = NULL;
	delete[] plane_h_raw;    plane_h_raw = NULL;
	delete[] RCS_raw;   RCS_raw = NULL;
    delete[] RCS_raw_angle;RCS_raw_angle=NULL;
	delete[] plane_lati_intp; plane_lati_intp = NULL;
	delete[] plane_logn_intp; plane_logn_intp = NULL;

	delete[] intp_x_tar; intp_x_tar = NULL;


	delete[] azAngle; azAngle = NULL;
	delete[] ptAngle; ptAngle = NULL;

	delete[] plane_h_intp;    plane_h_intp = NULL;
	delete[] tar_h_intp;	   tar_h_intp = NULL;
	delete[] sltRange;        sltRange = NULL;

    return true;
}



/*
*/

bool gps_rcs_files_read_single(QString gpsfile,
	double tar_lon,
	double tar_lat,
	double tar_h,
	QString rcsfile,
	QVector<dataunit> &vec_data, 
    cTime& startTime,double &target_amuzh_angle)
	{
		
		
		
	if (vec_data.size() > 0)
	{
		vec_data.clear();
	}

	dataunit dataunit_temp;
	//cTime time_temp;
	//target
	

	// plane
	double *plane_lati_raw = new double[MAX_RCS_POINTS];
	double *plane_logn_raw = new double[MAX_RCS_POINTS];
	double *plane_h_raw = new double[MAX_RCS_POINTS];
	
	memset(plane_lati_raw, 0, sizeof(double)*MAX_RCS_POINTS);
	memset(plane_logn_raw, 0, sizeof(double)*MAX_RCS_POINTS);
	memset(plane_h_raw, 0, sizeof(double)*MAX_RCS_POINTS);





	//RCS
	double* RCS_raw = new double[MAX_RCS_POINTS];
    memset(RCS_raw,0,sizeof(double)*MAX_RCS_POINTS);

    double* RCS_angle=new double[MAX_RCS_POINTS];
    memset(RCS_angle,0,sizeof(double)*MAX_RCS_POINTS);



	//read plane GPS files  ====读取载机的GPS数据
	char line[1024];

	QByteArray tempArray = gpsfile.toLocal8Bit();
	char* pPath = tempArray.data();
	std::ifstream fin(pPath, std::ios::in);
	long plat_gps_point = 0;

	QStringList plat_time;
	QStringList plat_date;


    double  longi_temp,lati_temp,height_temp;

	while (fin.getline(line, sizeof(line)))
	{
		QString cmd = QString("%1").arg(line);
		if (cmd.isEmpty())continue;
		QStringList list;
		list = cmd.split(QRegExp("\\s+"));

		if (plat_gps_point >= MAX_RCS_POINTS) break;




        if(0==plat_gps_point)
        {
             longi_temp=list.at(2).toDouble();
             lati_temp= list.at(3).toDouble();
             height_temp=list.at(4).toDouble();

             plane_logn_raw[plat_gps_point] = list.at(2).toDouble();
             plane_lati_raw[plat_gps_point] = list.at(3).toDouble();
             plane_h_raw[plat_gps_point] = list.at(4).toDouble();
             plat_gps_point++;
             plat_time.append(list.at(1));
             plat_date.append(list.at(0));
        }



        if(list.at(2).toDouble()!=longi_temp||list.at(3).toDouble()!=lati_temp)
       {
            plane_logn_raw[plat_gps_point] = list.at(2).toDouble();
            plane_lati_raw[plat_gps_point] = list.at(3).toDouble();
            plane_h_raw[plat_gps_point] = list.at(4).toDouble();
            plat_gps_point++;

            plat_time.append(list.at(1));
            plat_date.append(list.at(0));

            longi_temp=list.at(2).toDouble();
              lati_temp= list.at(3).toDouble();
                height_temp=list.at(4).toDouble();
        }


	}



	fin.close();

	if (plat_gps_point == 0)
	{
		//QMessageBox::critical(this, tr("错误"), tr("飞机GPS文件无法读取！"));
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

	bool bSAR = false;
	long rcs_points = 0;
	double RCS_max = -9999.0;
	double RCS_max_angle = 0;
	if (rcsfile.endsWith(".jpg", Qt::CaseInsensitive))
		bSAR = true;

	if (!bSAR)
	{
		// 读取RCS文件，获取总点数
		std::ifstream fin2(rcsfile.toLocal8Bit().data(), std::ios::in);
		
		while (fin2.getline(line, sizeof(line)))
		{
			QString cmd = QString("%1").arg(line);
			if (cmd.isEmpty())continue;
			QStringList list;
			list = cmd.split(QRegExp("\\s+"), QString::SkipEmptyParts);
			RCS_raw[rcs_points] = list.at(1).toDouble();
			RCS_angle[rcs_points] = list.at(0).toDouble();
			if (RCS_raw[rcs_points] > RCS_max)
			{
				RCS_max = RCS_raw[rcs_points];
				RCS_max_angle = RCS_angle[rcs_points];
			}

			if (rcs_points >= MAX_RCS_POINTS) break;
			rcs_points++;
		}

		fin2.close();
		if (rcs_points <= 0)
		{
			//QMessageBox::critical(this, tr("错误"), tr("无法从RCS文件读取数据！"));
			return false;
		}
	}

	// all above is ok
	// 插值tar_points,plat_gps_point,rcs_points个点数
    long N = plat_gps_point;
	

	// target
    //double *tar_lati_intp = new double[N];
    //double *tar_logn_intp = new double[N];
    //double *tar_h_intp = new double[N];
    //double *tar_hx_intp = new double[N];

	// plane
	double *plane_lati_intp = new double[N];
	double *plane_logn_intp = new double[N];
	double *plane_h_intp = new double[N];
    double *Plane_time_sec_intp=new double[N];
	
for(int k=0;k<N;k++)
{


    plane_lati_intp[k]=plane_lati_raw[k];
    plane_logn_intp[k]=plane_logn_raw[k];
    plane_h_intp[k]=plane_h_raw[k];
    Plane_time_sec_intp[k]=plat_sta_sec+(plat_end_sec-plat_sta_sec)*k/(N-1);
}

	
	




	// 计算方位角并保存；
    double *azAngle = new double[N];
    double *ptAngle = new double[N];
    double *sltRange = new double[N];
    memset(azAngle, 0, sizeof(double)*N);
    memset(ptAngle, 0, sizeof(double)*N);
    memset(sltRange, 0, sizeof(double)*N);



    int RCS_index;

    for (long k = 0; k < N; k++)
	{

        double X_lati = (plane_lati_intp[k] - tar_lat)*111693.1;  // 纬度信息地面投影
        double Y_logn = (plane_logn_intp[k] - tar_lon)*111693.1*cos(plane_lati_intp[k] * pi / 180.0); //经度信息地面投影

        double high = (plane_h_intp[k] - tar_h); // 载机到地面的高度
		sltRange[k] = sqrt(high*high + X_lati*X_lati + Y_logn*Y_logn); // 载机到目标的斜距

		ptAngle[k] = 90 - acos(high / sltRange[k]) * 180 / pi; // 与水平面的夹角为俯仰角;
		azAngle[k] = atan2(Y_logn, X_lati) * 180 / pi;           // 与正北方向的夹角为定义的方位角
		if (azAngle[k] < 0) azAngle[k] += 360;

        //azAngle[k] -= tar_hx_intp[k];  // 计算相对于船的航向的夹角；

		while (azAngle[k] < 0)	{ azAngle[k] += 360; }
		while (azAngle[k] > 360) { azAngle[k] -= 360; } // 添加2017/3/25;

		dataunit_temp.angle = azAngle[k];
		if (bSAR)
		{
			dataunit_temp.RCS_dB = 0.0;
		}
		else
		{
			RCS_index = FindAngleIndex(RCS_angle, dataunit_temp.angle, rcs_points);
			dataunit_temp.RCS_dB = RCS_raw[RCS_index];
		}

		dataunit_temp.plane_lon = plane_logn_intp[k];
		dataunit_temp.plane_lat = plane_lati_intp[k];
		dataunit_temp.plane_Height = plane_h_intp[k];
        dataunit_temp.target_lon = tar_lon;
        dataunit_temp.target_lat = tar_lat;
        dataunit_temp.target_Height = tar_h;
        dataunit_temp.dTime = Plane_time_sec_intp[k];


		vec_data.push_back(dataunit_temp);
	}


    target_amuzh_angle=90-RCS_max_angle;
    while(target_amuzh_angle<0)
        target_amuzh_angle=target_amuzh_angle+360;
    while(target_amuzh_angle>360)
        target_amuzh_angle=target_amuzh_angle-360;


	delete[] plane_lati_raw; plane_lati_raw = NULL;
	delete[] plane_logn_raw; plane_logn_raw = NULL;
	delete[] plane_h_raw;    plane_h_raw = NULL;
	delete[] RCS_raw;   RCS_raw = NULL;

    delete [] RCS_angle; RCS_angle=NULL;

	delete[] plane_lati_intp; plane_lati_intp = NULL;
	delete[] plane_logn_intp; plane_logn_intp = NULL;
    delete [] Plane_time_sec_intp;Plane_time_sec_intp=NULL;

	delete[] azAngle; azAngle = NULL;
	delete[] ptAngle; ptAngle = NULL;

	delete[] plane_h_intp;    plane_h_intp = NULL;

	delete[] sltRange;        sltRange = NULL;

	return true;
		
		
		
		
		
		
	}
