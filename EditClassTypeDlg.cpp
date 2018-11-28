#include "EditClassTypeDlg.h"
#include "DataType.h"
#include <QJsonArray>
#include <QDateEdit>
#include <QComboBox>
#include <QMessageBox>
#include <QUuid>  
#include <QSqlQuery>

EditClassTypeDlg::EditClassTypeDlg(const QJsonObject& obj, bool bEdit, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	setWindowTitle(QString::fromLocal8Bit("修改类型"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowIcon(QIcon());
	
	m_jsonObj = obj;
	m_strTableName = m_jsonObj.value(Class_Guid).toString();

	QString strClassAnno = m_jsonObj.value(Class_Anno).toString();
	QJsonArray arrayField = m_jsonObj.value(Class_Fields).toArray();
	int nRows = arrayField.size();

	ui.lineEdit_ClassAnno->setText(m_jsonObj.value(Class_Anno).toString());
	
	if (!bEdit)
	{
		ui.lineEdit_ClassAnno->setReadOnly(true);
	}

	ui.tableWidget->setColumnCount(3);
	ui.tableWidget->setRowCount(nRows);

	QStringList strHeader;
	strHeader << QString::fromLocal8Bit("描述") << QString::fromLocal8Bit("数据类型") << QString::fromLocal8Bit("");
	ui.tableWidget->setHorizontalHeaderLabels(strHeader);

	for (int i = 0; i < nRows; i++)
	{
		QJsonObject objRecord = arrayField.at(i).toObject();
		QString strFieldName = objRecord.value(Field_Name).toString();
		QString strFieldType = objRecord.value(Field_DataType).toString();

		QTableWidgetItem* pItem1 = new QTableWidgetItem(strFieldName);
		//pItem1->setFlags(pItem1->flags() & (~Qt::ItemIsEditable));

// 		QTableWidgetItem* pItem2 = new QTableWidgetItem(strFieldType);
// 		pItem2->setFlags(pItem2->flags() & (~Qt::ItemIsEditable));

		ui.tableWidget->setItem(i, 0, pItem1);
//		ui.tableWidget->setItem(i, 1, pItem2);

		QComboBox* pComboBox = new QComboBox;
		for (int x = 0; x < FieldType::MaxCount; x++)
		{
			pComboBox->addItem(DataType::typeDescription(x));
		}

		pComboBox->setCurrentText(strFieldType);

		ui.tableWidget->setCellWidget(i, 1, pComboBox);
		
		if (!bEdit)
			pComboBox->setEnabled(false);

		QString strTemp;
		if (bEdit)
		{
			strTemp = QString::fromLocal8Bit("删除");
		}
		QTableWidgetItem* pItem = new QTableWidgetItem(strTemp);
		pItem->setFlags(pItem->flags() & (~Qt::ItemIsEditable));
		ui.tableWidget->setItem(i, 2, pItem);
	}
	
	QObject::connect(ui.pushButton_AddField, SIGNAL(clicked()), this, SLOT(slotAddField()));

	//SQLite 不支持 “drop column”，即不支持删除字段
	QObject::connect(ui.tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotDeleteCell(int, int)));

	QObject::connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	resize(600, 400);
}

EditClassTypeDlg::~EditClassTypeDlg()
{

}

void EditClassTypeDlg::slotAddField()
{
	int nRows = ui.tableWidget->rowCount();
	ui.tableWidget->insertRow(nRows);

	QComboBox* pComboBox = new QComboBox;
	for (int i = 0; i < FieldType::MaxCount; i++)
	{
		pComboBox->addItem(DataType::typeDescription(i));
	}

	ui.tableWidget->setCellWidget(nRows, 1, pComboBox);
	QTableWidgetItem* pItem = new QTableWidgetItem(QString::fromLocal8Bit("删除"));
	pItem->setFlags(pItem->flags() & (~Qt::ItemIsEditable));
	ui.tableWidget->setItem(nRows, 2, pItem);
}

void EditClassTypeDlg::slotDeleteCell(int nRow, int nCol)
{
	if (nCol != 2)
		return;

	QTableWidgetItem* pItem = ui.tableWidget->item(nRow, nCol);
	QString strColumnName = pItem->text();
	if (strColumnName.isEmpty())
		return;

	ui.tableWidget->removeRow(nRow);
	
// 	QString sqlDeleteColumn = "ALTER TABLE " + m_strTableName + " DROP COLUMN " + strColumnName;
// 
// 	QSqlQuery sql_query;
// 	sql_query.prepare(sqlDeleteColumn);
// 	sql_query.exec();
}

void EditClassTypeDlg::accept()
{
	int nRows = ui.tableWidget->rowCount();
	if (nRows <= 0)
	{
		QMessageBox box;
		box.setText(QString::fromLocal8Bit("字段不能为空"));
		box.exec();
		return;
	}

	QString strClassAnno = ui.lineEdit_ClassAnno->text();
	if (strClassAnno.isEmpty() || strClassAnno.isNull())
	{
		QMessageBox box;
		box.setText(QString::fromLocal8Bit("类名不能为空"));
		box.exec();
		return;
	}

	for (int i = 0; i < nRows; i++)
	{
		QTableWidgetItem* pItem = ui.tableWidget->item(i, 0);
		if (pItem == nullptr)
		{
			QMessageBox box;
			box.setText(QString::fromLocal8Bit("输入不能为空"));
			box.exec();
			return;
		}

		QString strDescription = pItem->text();
		if (strDescription.isEmpty() || strDescription.isNull())
		{
			QMessageBox box;
			box.setText(QString::fromLocal8Bit("输入不能为空"));
			box.exec();
			return;
		}
	}

	m_jsonObj = QJsonObject();
	m_jsonObj.insert(Class_Anno, strClassAnno);
	QJsonArray jsonArray;
	for (int i = 0; i < nRows; i++)
	{
		QJsonObject fieldObj;
		fieldObj.insert(Field_Name, ui.tableWidget->item(i, 0)->text());

		QComboBox* pComboBox = dynamic_cast<QComboBox*>(ui.tableWidget->cellWidget(i, 1));
		fieldObj.insert(Field_DataType, pComboBox->currentText());

		jsonArray.append(fieldObj);
	}

	m_jsonObj.insert(Class_Fields, jsonArray);
	m_jsonObj.insert(Class_Guid, m_strTableName);

	QDialog::accept();
}

void EditClassTypeDlg::reject()
{
	QDialog::reject();
}

QJsonObject EditClassTypeDlg::jsonObject()
{
	return m_jsonObj;
}