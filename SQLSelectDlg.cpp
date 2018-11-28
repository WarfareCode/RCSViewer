#include "SQLSelectDlg.h"
#include <QJsonArray>
#include "DataType.h"
#include "DateInputDlg.h"

SQLSelectDlg::SQLSelectDlg(const QJsonObject& classObj, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("≤È—Ø"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	m_jsonObj = classObj;

	QJsonArray arrayField = m_jsonObj.value(Class_Fields).toArray();
	int nRows = arrayField.size();

	for (int i = 0; i < nRows; i++)
	{
		QJsonObject objRecord = arrayField.at(i).toObject();
		QString strFieldName = objRecord.value(Field_Name).toString();
		QPushButton* pButton = new QPushButton;
		pButton->setText(strFieldName);
		ui.gridLayout_Field->addWidget(pButton, i / 3, i % 3);
		connect(pButton, SIGNAL(clicked()), this, SLOT(slotFieldButtonClicked()));
	}

	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(slotFieldButtonClicked()));
	connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(slotFieldButtonClicked()));
	connect(ui.pushButton_3, SIGNAL(clicked()), this, SLOT(slotFieldButtonClicked()));
	connect(ui.pushButton_4, SIGNAL(clicked()), this, SLOT(slotFieldButtonClicked()));
	connect(ui.pushButton_5, SIGNAL(clicked()), this, SLOT(slotFieldButtonClicked()));
	connect(ui.pushButton_6, SIGNAL(clicked()), this, SLOT(slotFieldButtonClicked()));
	connect(ui.pushButton_7, SIGNAL(clicked()), this, SLOT(slotFieldButtonClicked()));
	connect(ui.pushButton_8, SIGNAL(clicked()), this, SLOT(slotFieldButtonClicked()));
	connect(ui.pushButton_9, SIGNAL(clicked()), this, SLOT(slotInputDate()));

	QObject::connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

SQLSelectDlg::~SQLSelectDlg()
{

}

void SQLSelectDlg::slotFieldButtonClicked()
{
	QPushButton* pButton = qobject_cast<QPushButton*>(sender());
	QString strText = pButton->text();
	//ui.textEdit->setText(ui.textEdit->toPlainText() + strText);
	ui.textEdit->insertPlainText(strText);
}

void SQLSelectDlg::slotInputDate()
{
	DateInputDlg dlg;
	if (dlg.exec())
	{
		QString strDateTime = dlg.date().toString("yyyy-MM-dd");
		strDateTime = "'" + strDateTime + "'";
		//ui.textEdit->setText(ui.textEdit->toPlainText() + strDateTime);
		ui.textEdit->insertPlainText(strDateTime);
	}
}

QString SQLSelectDlg::SQLString()
{
	return ui.textEdit->toPlainText();
}

void SQLSelectDlg::accept()
{
	QDialog::accept();
}

void SQLSelectDlg::reject()
{
	QDialog::reject();
}