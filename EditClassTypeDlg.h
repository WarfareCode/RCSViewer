#ifndef EDITCLASSTYPEDLG_H
#define EDITCLASSTYPEDLG_H

#include <QDialog>
#include <QJsonObject>
#include "ui_EditClassTypeDlg.h"

class EditClassTypeDlg : public QDialog
{
	Q_OBJECT

public:
	EditClassTypeDlg(const QJsonObject& obj, bool bEdit = false, QWidget *parent = 0);
	~EditClassTypeDlg();

	QJsonObject jsonObject();

public slots:

	void slotAddField();

	void slotDeleteCell(int nRow, int nCol);

	void accept();
	void reject();

private:
	Ui::EditClassTypeDlg ui;

	QJsonObject m_jsonObj;

	QString m_strTableName;
};

#endif // EDITCLASSTYPEDLG_H
