#include "AddClassRecordsDlg.h"
#include "DataType.h"
#include <QJsonArray>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include "ValueInputDlg.h"
#include "DateInputDlg.h"
#include "OBEnumInputDlg.h"
#include <QMessageBox>
#include "Document.h"

extern bool GetInputValue(QString strType, QWidget* pWidget, QJsonValue& value);

AddClassRecordsDlg::AddClassRecordsDlg(const QString& strClassFile, const QJsonObject& obj, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("批量添加记录"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	m_jsonObj = obj;
	m_strClassFile = strClassFile;

	QString strClassAnno = m_jsonObj.value(Class_Anno).toString();
	QJsonArray arrayField = m_jsonObj.value(Class_Fields).toArray();
	int nCols = arrayField.size();
	QStringList strHeader;

	for (int i = 0; i < nCols; i++)
	{
		QJsonObject objRecord = arrayField.at(i).toObject();
		QString strFieldName = objRecord.value(Field_Name).toString();
		strHeader << strFieldName;
	}

	ui.tableWidget->setColumnCount(nCols);
	ui.tableWidget->setHorizontalHeaderLabels(strHeader);

 	connect(ui.tableWidget->horizontalHeader(), SIGNAL(sectionClicked(int)),
 		this, SLOT(slotHeaderClicked(int)));

	QObject::connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	QObject::connect(ui.tableWidget, SIGNAL(cellClicked(int, int)), SLOT(OnCellClicked(int, int)));
}

AddClassRecordsDlg::~AddClassRecordsDlg()
{

}

void AddClassRecordsDlg::slotHeaderClicked(int nIndex)
{
	QJsonArray arrayField = m_jsonObj.value(Class_Fields).toArray();
	QJsonObject objRecord = arrayField.at(nIndex).toObject();
	QString strFieldType = objRecord.value(Field_DataType).toString();
	
	if (strFieldType.compare(FieldType_Int_Des) == 0 
		|| strFieldType.compare(FieldType_Double_Des) == 0
		|| strFieldType.compare(FieldType_String_Des) == 0)
	{
		ValueInputDlg dlg(strFieldType);
		if (dlg.exec())
		{
			QString strValue = dlg.editText();
			int nRowCount = ui.tableWidget->rowCount();
			for (int i = 0; i < nRowCount; i++)
			{
				QLineEdit* pEdit = (QLineEdit*)ui.tableWidget->cellWidget(i, nIndex);
				pEdit->setText(strValue);
			}
		}
	}
	else if (strFieldType.compare(FieldType_Bool_Des) == 0)
	{
		ValueInputDlg dlg(strFieldType);
		if (dlg.exec())
		{
			QString strValue = dlg.editText();
			int nRowCount = ui.tableWidget->rowCount();
			for (int i = 0; i < nRowCount; i++)
			{
				QComboBox* pComboBox = (QComboBox*)ui.tableWidget->cellWidget(i, nIndex);
				int nComboxIndex = strValue.toInt();
				pComboBox->setCurrentIndex(nComboxIndex);
			}
		}
	}
	else if (strFieldType.compare(FieldType_DateTime_Des) == 0)
	{
		DateInputDlg dlg;
		if (dlg.exec())
		{
			QDate editDate = dlg.date();
			int nRowCount = ui.tableWidget->rowCount();
			for (int i = 0; i < nRowCount; i++)
			{
				QDateEdit* pDateEdit = (QDateEdit*)ui.tableWidget->cellWidget(i, nIndex);
				pDateEdit->setDate(editDate);
			}
		}
	}
	else if (strFieldType.compare(FieldType_OBMode_Des) == 0 
		|| strFieldType.compare(FieldType_OBTarget_Des) == 0
		|| strFieldType.compare(FieldType_Polarity_Des) == 0)
	{
		OBEnumInputDlg dlg(strFieldType);
		if (dlg.exec())
		{
			QString strValue = dlg.text();
			int nRowCount = ui.tableWidget->rowCount();
			for (int i = 0; i < nRowCount; i++)
			{
				QComboBox* pComboBox = (QComboBox*)ui.tableWidget->cellWidget(i, nIndex);
				pComboBox->setCurrentText(strValue);
			}
		}
	}
	else if (strFieldType.compare(FieldType_File_Des) == 0)
	{
		QStringList listFiles = QFileDialog::getOpenFileNames(this);
		if (listFiles.isEmpty())
			return;

		int nFileIndex = 0;
		int nFileCount = listFiles.size();
		int nRowCount = ui.tableWidget->rowCount();
		for (int i = 0; i < nRowCount; i ++)
		{
			QLabel* pLabel = (QLabel*)ui.tableWidget->cellWidget(i, nIndex);
			if (pLabel->text().isEmpty())
			{
				if (nFileIndex >= nFileCount)
					break;

				pLabel->setText(listFiles[nFileIndex]);
				nFileIndex++;
			}
		}

		for (int i = nFileIndex; i < nFileCount; i++)
		{
			int nSize = ui.tableWidget->rowCount();
			ui.tableWidget->insertRow(nSize);

			int nFieldSize = arrayField.size();
			for (int j = 0; j < nFieldSize; j ++)
			{
				QJsonObject objRecord1 = arrayField.at(j).toObject();
				QString strFieldType1 = objRecord1.value(Field_DataType).toString();

				if (strFieldType1.compare(FieldType_Int_Des) == 0)
				{
					QLineEdit* pEdit = new QLineEdit;
					pEdit->setValidator(new QIntValidator);
					ui.tableWidget->setCellWidget(nSize, j, pEdit);
				}
				else if (strFieldType1.compare(FieldType_Double_Des) == 0)
				{
					QLineEdit* pEdit = new QLineEdit;
					pEdit->setValidator(new QDoubleValidator);
					ui.tableWidget->setCellWidget(nSize, j, pEdit);
				}
				else if (strFieldType1.compare(FieldType_Bool_Des) == 0)
				{
					QComboBox* pBox = new QComboBox;
					pBox->addItem("0");
					pBox->addItem("1");
					ui.tableWidget->setCellWidget(nSize, j, pBox);
				}
				else if (strFieldType1.compare(FieldType_OBMode_Des) == 0
					|| strFieldType1.compare(FieldType_OBTarget_Des) == 0
					|| strFieldType1.compare(FieldType_Polarity_Des) == 0)
				{
					QComboBox* pBox = new QComboBox;
					for (int m = 0; m < DataType::enumTypeCount(strFieldType1); m++)
					{
						pBox->addItem(DataType::enumTypeDescription(strFieldType1, m));
					}

					ui.tableWidget->setCellWidget(nSize, j, pBox);
				}
				else if (strFieldType1.compare(FieldType_String_Des) == 0)
				{
					ui.tableWidget->setCellWidget(nSize, j, new QLineEdit);
				}
				else if (strFieldType1.compare(FieldType_File_Des) == 0)
				{
					ui.tableWidget->setCellWidget(nSize, j, new QLabel);
				}
				else if (strFieldType1.compare(FieldType_DateTime_Des) == 0)
				{
					QDateEdit* dateEdit = new QDateEdit();
					dateEdit->setCalendarPopup(true);
					dateEdit->setDate(QDate::currentDate());

					ui.tableWidget->setCellWidget(nSize, j, dateEdit);
				}
			}
			QLabel* pLabel = (QLabel*)ui.tableWidget->cellWidget(nSize, nIndex);
			pLabel->setText(listFiles[i]);
		}
	}
}

QJsonArray AddClassRecordsDlg::recordsJson()
{
	return m_jsonArrayRecords;
}

void AddClassRecordsDlg::OnCellClicked(int nRow, int nCol)
{
	QFileDialog fileDlg;
	QString strPath = fileDlg.getOpenFileName();

	if (strPath.isEmpty())
		return;

	QLabel* pLabel = (QLabel*)ui.tableWidget->cellWidget(nRow, nCol);
	pLabel->setText(strPath);
}

void AddClassRecordsDlg::accept()
{
	m_jsonArrayRecords = QJsonArray();
 	QJsonArray arrayField = m_jsonObj.value(Class_Fields).toArray();

	int nRowCount = ui.tableWidget->rowCount();
	int nColumnCount = ui.tableWidget->columnCount();

	QStringList strFiles;

	bool bAllValid = true;
	for (int j = 0; j < nRowCount; j ++)
	{
		QJsonArray record;
		for (int i = 0; i < nColumnCount; i ++)
		{
			QWidget* pWidget = ui.tableWidget->cellWidget(j, i);
			QJsonObject objRecord = arrayField.at(i).toObject();
			QString strFieldType = objRecord.value(Field_DataType).toString();

			if (strFieldType == FieldType_File_Des)
			{
				QLabel* pLabel = (QLabel*)pWidget;
				QString strLabel = pLabel->text();

				if (!strLabel.isEmpty())
				{
					strFiles.push_back(strLabel);
				}
			}

			QJsonValue value;
			if (!GetInputValue(strFieldType, pWidget, value))
			{
				bAllValid = false;
				break;
			}
			record.append(value);
		}

		m_jsonArrayRecords.append(record);
	}

	if (!bAllValid)
	{
		QMessageBox box;
		box.setText(QString::fromLocal8Bit("输入不能为空"));
		box.exec();
		return;
	}

	if (Document::needCopyFile())
	{
		//判断输入文件里是否有重复
		for (int i = 0; i < strFiles.size() - 1; i++)
		{
			for (int j = i + 1; j < strFiles.size(); j++)
			{
				if (strFiles[i].compare(strFiles[j], Qt::CaseInsensitive) == 0)
				{
					QMessageBox box;
					box.setWindowTitle(QString::fromLocal8Bit("提示"));
					box.setText(QString::fromLocal8Bit("有同名文件，请重新输入"));
					box.exec();
					return;
				}
			}
		}

		//判断目标文件夹是否已经有同名文件
		QString strDesDir = QFileInfo(m_strClassFile).absolutePath();
		strDesDir.remove(Document::projectDir());
		strDesDir.remove(0, 5);
		strDesDir = "/data" + strDesDir;
		strDesDir = Document::projectDir() + strDesDir;

		QDir dirAll(strDesDir);
		dirAll.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
		QFileInfoList strList = dirAll.entryInfoList();

		QString strSrcFileName;

		for (QFileInfoList::iterator itr = strList.begin(); itr != strList.end(); itr++)
		{
			QFileInfo& fileInfo = *itr;

			for (int i = 0; i < strFiles.size(); i++)
			{
				strSrcFileName = QFileInfo(strFiles[i]).fileName();
				if (fileInfo.fileName().compare(strSrcFileName, Qt::CaseInsensitive) == 0)
				{
					QMessageBox box;
					box.setWindowTitle(QString::fromLocal8Bit("提示"));
					box.setText(QString::fromLocal8Bit("包含同名文件或者该文件已经入库，请重命名：") + strSrcFileName);
					box.exec();
					return;
				}
			}
		}
	}

	QDialog::accept();
}

void AddClassRecordsDlg::reject()
{
	QDialog::reject();
}