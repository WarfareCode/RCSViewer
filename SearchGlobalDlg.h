#ifndef SEARCHGLOBALDLG_H
#define SEARCHGLOBALDLG_H

#include <QDialog>
#include "ui_SearchGlobalDlg.h"

class SearchGlobalDlg : public QDialog
{
	Q_OBJECT

public:
	SearchGlobalDlg(QStringList listRecentProject, QWidget *parent = 0);
	~SearchGlobalDlg();

public slots:

	void slotSearchGlobal();

	void slotShow3D();

private:
	Ui::SearchGlobalDlg ui;

	QStringList m_listRecentProject;

	QStringList m_listTableName;

	QMap<QString, QJsonObject>m_mapAllClassInfo;
};

#endif // SEARCHGLOBALDLG_H
