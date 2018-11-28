#ifndef ADDCLASSRECORDDLG_H
#define ADDCLASSRECORDDLG_H

#include <QDialog>
#include <QJsonObject>
#include <QJsonArray>
#include "ui_AddClassRecordDlg.h"

class AddClassRecordDlg : public QDialog
{
	Q_OBJECT

public:
	AddClassRecordDlg(const QJsonObject& obj, QWidget *parent = 0);
	~AddClassRecordDlg();

	const QJsonArray& recordJson();

public slots:

	void accept();
	void reject();

	void OnCellClicked(int nRow, int nCol);

private:
	Ui::AddClassRecordDlg ui;

	QJsonObject m_jsonObj;

	QJsonArray m_jsonRecord;
};

#endif // ADDCLASSRECORDDLG_H
