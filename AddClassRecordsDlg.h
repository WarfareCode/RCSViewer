#ifndef ADDCLASSRECORDSDLG_H
#define ADDCLASSRECORDSDLG_H

#include <QDialog>
#include "ui_AddClassRecordsDlg.h"
#include <QJsonObject>
#include <QJsonArray>

class AddClassRecordsDlg : public QDialog
{
	Q_OBJECT

public:
	AddClassRecordsDlg(const QString& strClassFile, const QJsonObject& obj, QWidget *parent = 0);
	~AddClassRecordsDlg();

	QJsonArray recordsJson();

public slots:

	void slotHeaderClicked(int nIndex);

	void accept();
	void reject();

	void OnCellClicked(int nRow, int nCol);

protected:


private:
	Ui::AddClassRecordsDlg ui;

	QJsonObject m_jsonObj;

	QJsonArray m_jsonArrayRecords;

	QString m_strClassFile;
};

#endif // ADDCLASSRECORDSDLG_H
