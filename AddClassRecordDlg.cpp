#include "AddClassRecordDlg.h"
#include "DataType.h"
#include <QJsonArray>
#include <QPushButton>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

bool GetInputValue(QString strType, QWidget* pWidget, QJsonValue& value)
{
	if (strType == FieldType_Double_Des
		|| strType == FieldType_Int_Des
		|| strType == FieldType_String_Des)
	{
		QLineEdit* pEdit = dynamic_cast<QLineEdit*>(pWidget);
		QString strLineEdit = pEdit->text();
		if (strLineEdit.isEmpty() || strLineEdit.isNull())
			return false;

		if (strType == FieldType_Double_Des)
		{
			value = strLineEdit.toDouble();
		}
		else if (strType == FieldType_Int_Des)
		{
			value = strLineEdit.toInt();
		}
		else if (strType == FieldType_String_Des)
		{
			value = strLineEdit;
		}
	}
	else if (strType == FieldType_File_Des)
	{
		QLabel* pLabel = (QLabel*)pWidget;
		QString strLabel = pLabel->text();

		if (strLabel.isEmpty() || strLabel.isNull())
		{
			return false;
		}

		value = strLabel;
	}
	else if (strType == FieldType_DateTime_Des)
	{
		QDateEdit* pEdit = (QDateEdit*)pWidget;
		QString strDateTime = pEdit->dateTime().toString("yyyy-MM-dd");
		value = strDateTime;
	}
	else if (strType == FieldType_Bool_Des)
	{
		QComboBox* pBox = (QComboBox*)pWidget;
		value = pBox->currentText().toInt();
	}
	else if (strType == FieldType_OBMode_Des 
		|| strType == FieldType_OBTarget_Des
		|| strType == FieldType_Polarity_Des)
	{
		QComboBox* pBox = (QComboBox*)pWidget;
		value = pBox->currentText();
	}

	return true;
}

AddClassRecordDlg::AddClassRecordDlg(const QJsonObject& obj, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("添加记录"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	m_jsonObj = obj;

	QString strClassAnno = m_jsonObj.value(Class_Anno).toString();
	QJsonArray arrayField = m_jsonObj.value(Class_Fields).toArray();
	int nRows = arrayField.size();

	ui.tableWidget->setColumnCount(3);
	ui.tableWidget->setRowCount(nRows);

	QStringList strHeader;
	strHeader << QString::fromLocal8Bit("描述") << QString::fromLocal8Bit("数据类型") << QString::fromLocal8Bit("值");
	ui.tableWidget->setHorizontalHeaderLabels(strHeader);

	for (int i = 0; i < nRows; i ++)
	{
		QJsonObject objRecord = arrayField.at(i).toObject();
		QString strFieldName = objRecord.value(Field_Name).toString();
		QString strFieldType = objRecord.value(Field_DataType).toString();

		QTableWidgetItem* pItem1 = new QTableWidgetItem(strFieldName);
		pItem1->setFlags(pItem1->flags() & (~Qt::ItemIsEditable));

		QTableWidgetItem* pItem2 = new QTableWidgetItem(strFieldType);
		pItem2->setFlags(pItem2->flags() & (~Qt::ItemIsEditable));

		ui.tableWidget->setItem(i, 0, pItem1);
		ui.tableWidget->setItem(i, 1, pItem2);

		if (strFieldType.compare(FieldType_Int_Des) == 0)
		{
			QLineEdit* pEdit = new QLineEdit;
			pEdit->setValidator(new QIntValidator);
			ui.tableWidget->setCellWidget(i, 2, pEdit);
		}
		else if (strFieldType.compare(FieldType_Double_Des) == 0)
		{
			QLineEdit* pEdit = new QLineEdit;
			pEdit->setValidator(new QDoubleValidator);
			ui.tableWidget->setCellWidget(i, 2, pEdit);
		}
		else if (strFieldType.compare(FieldType_Bool_Des) == 0)
		{
			QComboBox* pBox = new QComboBox;
			pBox->addItem("0");
			pBox->addItem("1");
			ui.tableWidget->setCellWidget(i, 2, pBox);
		}
		else if (strFieldType.compare(FieldType_String_Des) == 0)
		{
			ui.tableWidget->setCellWidget(i, 2, new QLineEdit);
		}
		else if (strFieldType.compare(FieldType_File_Des) == 0)
		{
			ui.tableWidget->setCellWidget(i, 2, new QLabel);
		}
		else if (strFieldType.compare(FieldType_DateTime_Des) == 0)
		{
			QDateEdit* dateEdit = new QDateEdit();
			dateEdit->setCalendarPopup(true);
			dateEdit->setDate(QDate::currentDate());

			ui.tableWidget->setCellWidget(i, 2, dateEdit);
		}
		else if (strFieldType.compare(FieldType_OBMode_Des) == 0
			|| strFieldType.compare(FieldType_OBTarget_Des) == 0
			|| strFieldType.compare(FieldType_Polarity_Des) == 0)
		{
			QComboBox* pBox = new QComboBox;
			for (int m = 0; m < DataType::enumTypeCount(strFieldType); m++)
			{
				pBox->addItem(DataType::enumTypeDescription(strFieldType, m));
			}

			ui.tableWidget->setCellWidget(i, 2, pBox);
		}
	}

	QObject::connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	QObject::connect(ui.tableWidget, SIGNAL(cellClicked(int, int)), SLOT(OnCellClicked(int, int)));

	resize(600, 400);
}

AddClassRecordDlg::~AddClassRecordDlg()
{

}

const QJsonArray& AddClassRecordDlg::recordJson()
{
	return m_jsonRecord;
}

void AddClassRecordDlg::accept()
{
	m_jsonRecord = QJsonArray();
	bool bAllValid = true;
	int nRows = ui.tableWidget->rowCount();
	for (int i = 0; i < nRows; i++)
	{
		QWidget* pWidget = ui.tableWidget->cellWidget(i, 2);
		QTableWidgetItem* pItem = ui.tableWidget->item(i, 1);
		QString strType = pItem->text();

		QJsonValue value;
		if (!GetInputValue(strType, pWidget, value))
		{
			bAllValid = false;
			break;
		}
		m_jsonRecord.append(value);
	}

	if (!bAllValid)
	{
		QMessageBox box;
		box.setText(QString::fromLocal8Bit("输入不能为空"));
		box.exec();
		return;
	}

	QDialog::accept();
}

void AddClassRecordDlg::reject()
{
	QDialog::reject();
}

void AddClassRecordDlg::OnCellClicked(int nRow, int nCol)
{
	if (nCol != 2)
		return;

	QFileDialog fileDlg;
	QString strPath = fileDlg.getOpenFileName();

	if (strPath.isEmpty())
		return;

	QLabel* pLabel = (QLabel*)ui.tableWidget->cellWidget(nRow, nCol);
	pLabel->setText(strPath);
}
