#ifndef DISPLAYFILESELECTDLG_H
#define DISPLAYFILESELECTDLG_H

#include <QDialog>
#include "ui_DisplayFileSelectDlg.h"

class DisplayFileSelectDlg : public QDialog
{
	Q_OBJECT

public:

	DisplayFileSelectDlg(const QStringList& listFileTypeName, const QStringList& listFilePath, QWidget *parent = 0);
	~DisplayFileSelectDlg();

	void getFilePath(QString& strPathPlaneGPS, QString& strPathTargetGPS, QString& strPathRCS, QString& strPathVideo);

private:

	Ui::DisplayFileSelectDlg ui;

	QStringList m_listFileTypeName;
	QStringList m_listFilePath;
};

#endif // DISPLAYFILESELECTDLG_H
