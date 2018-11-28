#include "AddClassTypeDlg.h"
#include "DataType.h"
#include <QComboBox>
#include <QMessageBox>
#include <QJsonArray>
#include <QUuid>  

AddClassTypeDlg::AddClassTypeDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("添加类型"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowIcon(QIcon());

	ui.tableWidget->setColumnCount(3);
	QStringList strHeader;
	strHeader << QString::fromLocal8Bit("描述") << QString::fromLocal8Bit("数据类型") << QString::fromLocal8Bit("");
	ui.tableWidget->setHorizontalHeaderLabels(strHeader);

	QObject::connect(ui.pushButton_AddField, SIGNAL(clicked()), this, SLOT(slotAddField()));
	QObject::connect(ui.tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotDeleteCell(int, int)));

	QObject::connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	resize(600, 400);
}

AddClassTypeDlg::~AddClassTypeDlg()
{

}

void AddClassTypeDlg::slotAddField()
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

void AddClassTypeDlg::slotDeleteCell(int nRow, int nCol)
{
	if (nCol == 2)
		ui.tableWidget->removeRow(nRow);
}

void AddClassTypeDlg::accept()
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

	for (int i = 0; i < nRows; i ++)
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

	QUuid id = QUuid::createUuid();
	QString strId = id.toString();
	strId.remove(QChar('{'));
	strId.remove(QChar('}'));
	strId.remove(QChar('-'));
	m_jsonObj.insert(Class_Fields, jsonArray);
	m_jsonObj.insert(Class_Guid, "t" + strId);

	QDialog::accept();
}

void AddClassTypeDlg::reject()
{
	QDialog::reject();
}

QJsonObject AddClassTypeDlg::jsonObject()
{
	return m_jsonObj;
}