#ifndef ADDCLASSTYPEDLG_H
#define ADDCLASSTYPEDLG_H

#include <QDialog>
#include <QJsonObject>
#include "ui_AddClassTypeDlg.h"

class AddClassTypeDlg : public QDialog
{
	Q_OBJECT

public:
	AddClassTypeDlg(QWidget *parent = 0);
	~AddClassTypeDlg();

	QJsonObject jsonObject();

public slots:

	void slotAddField();

	void slotDeleteCell(int nRow, int nCol);

	void accept();
	void reject();

private:
	Ui::AddClassTypeDlg ui;

	QJsonObject m_jsonObj;
};

#endif // ADDCLASSTYPEDLG_H
